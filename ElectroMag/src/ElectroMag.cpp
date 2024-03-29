/*
 * Copyright (C) 2010 - Alexandru Gagniuc - <mr.nuke.me@gmail.com>
 * This file is part of ElectroMag.
 *
 * ElectroMag is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ElectroMag is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with ElectroMag.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#if !defined(__CYGWIN__) // Stupid, I know, but it's a fact of life
#include <omp.h>
#endif
#include "./../../GPGPU_Segment/src/CL_Manager.hpp"
#include "Electromag utils.h"
#include "Graphics_dynlink.h"
#include <SOA_utils.hpp>
#include <thread>

//using namespace std;
// Use float or double; 16-bit single will generate errors
#define FPprecision float

struct SimulationParams {
	const char *name;
	size_t nx; // Number of lines on the x direction
	size_t ny; // Number of lines on the y direction
	size_t nz; // Number of lines on the z direction
	size_t pStatic; // Number of static point charges
	size_t pDynamic; // Number of dynamic charge elements
	size_t len; // Number of steps of a field line
};

static const struct SimulationParams param_list[] = {
	{ "default", 128, 128, 1, 1024, 0, 2500 },
	{ "enhanced", 256, 112, 1, 2048, 0, 5000 },
	{ "extreme", 256, 256, 1, 2048, 0, 5000 },
	{ "insane", 512, 512, 1, 2048, 0, 5000 },
	{ "fuckinginsane", 1024, 1024, 1, 5120, 0, 10000 },
	{ "cpu", 64, 64, 1, 1000, 0, 1000 },
	{ "micro", 16, 16, 1, 1000, 0, 1000 },
	{ "bogo", 16, 16, 1, 50, 0, 500 },
	{ "" },
};

void TestCL(Vector3<Array<float> > &fieldLines,
	    Array<electro::pointCharge<float> > &pointCharges, size_t n,
	    float resolution, perfPacket &perfData, bool useCurvature,
	    const char *preferred_platform_name = "");

using std::cerr;
using std::cout;
using std::endl;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

static const char *strnext(const char *str, const char sep)
{
	const char *substring = strchr(str, sep);

	if (!substring)
		return "";

	return ++substring;
}

static bool starts_with(const char *str, const char *start)
{
	return str == strstr(str, start);
}

static const struct SimulationParams *get_sim_params(const char *name)
{
	const struct SimulationParams *sim = param_list;

	do {
		if (!strcmp(sim->name, name))
			return sim;

		sim++;
	} while (strcmp(sim->name, ""));

	return NULL;
}

static void print_simsize_help(const char *name)
{
	const struct SimulationParams *sim = param_list;

	cout << "Invalid similatin parameter '" << name << "'" << endl;
	cout << "Available options are";

	do {
		cout << " '" << sim->name << "'";
		sim++;
	} while (strcmp(sim->name, ""));

	cout << endl;
}

// to redirect stdout and stderr to out.txt use:
//              >out.txt  2>&1
int main(int argc, char *argv[])
{
	const char *sim_name, *cl_plat_name = NULL;

	cout << " Electromagnetism simulation application" << endl;
	cout << " Compiled on " << __DATE__ << " at " << __TIME__ << endl;

	OpenCL::GlobalClManager.ListAllDevices();

	SimulationParams simConfig = param_list[0];
	const SimulationParams *sim;
	bool CPUenable = false, GPUenable = true, display = true;
	bool useCurvature = true;
	bool randseed = false;
	bool randfieldinit = false;
	bool regressData = false;
	// OpenCL devel tests?
	bool clMode = false;
	// Get command-line options;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--cpu")) {
			CPUenable = true;
		} else if (!strcmp(argv[i], "--gpudisable")) {
			GPUenable = false;
			CPUenable = true;
		} else if (!strcmp(argv[i], "--nodisp")) {
			display = false;
		} else if (starts_with(argv[i], "--simsize")) {
			sim_name = strnext(argv[i], '=');
			sim = get_sim_params(sim_name);
			if (!sim) {
				print_simsize_help(sim_name);
				return EXIT_FAILURE;
			}
			simConfig = *sim;
		} else if (!strcmp(argv[i], "--randseed")) {
			randseed = true;
		} else if (!strcmp(argv[i], "--randfieldinit")) {
			randfieldinit = true;
		} else if (!strcmp(argv[i], "--autoregress")) {
			regressData = true;
		} else if (!strcmp(argv[i], "--clmode")) {
			clMode = true;
		} else if (starts_with(argv[i], "--clplatform")) {
			cl_plat_name = strnext(argv[i], '=');
		} else {
			cout << " Ignoring unknown argument: " << argv[i]
			     << endl;
		}
	}

	Render::Renderer *FieldDisplay = 0;
	// Do we need to load the graphicsModule?
	if (display) {
		Graphics::ModuleLoadCode errCode;
		errCode = Graphics::LoadModule();
		if (errCode != Graphics::SUCCESS) {
			cerr << " Could not load graphhics module. Rendering disabled"
			     << endl;
			display = false;
		} else {
			FieldDisplay = Graphics::CreateFieldRenderer();
		}
	}

	CPUID::CpuidString cpuString;
	CPUID::GetCpuidString(&cpuString);

	CPUID::CpuidFeatures cpuInfo;
	CPUID::GetCpuidFeatures(&cpuInfo);

	const char *support[2] = { "not supported", "supported" };

	//freopen("log.bs.txt", "w", stderr);
	std::clog << " Processor:\t";
	std::clog.write(cpuString.IDString, sizeof(cpuString.IDString));
	std::clog << endl;
	std::clog << " SSE3:  \t" << support[cpuInfo.SSE3] << endl;
	std::clog << " SSSE3: \t" << support[cpuInfo.SSSE3] << endl;
	std::clog << " SSE4.1:\t" << support[cpuInfo.SSE41] << endl;
	std::clog << " SSE4.2:\t" << support[cpuInfo.SSE42] << endl;
	std::clog << " AVX256:\t" << support[cpuInfo.AVX] << endl;

	GPUenable = false;
	CPUenable = true;
	// Statistics show that users are happier when the program outputs fun
	// information abot their toys

	// Initialze data containers
	size_t nw = (int)simConfig.nx, nh = (int)simConfig.ny,
	       nd = (int)simConfig.nz, n = nh * nw * nd,
	       p = (int)simConfig.pStatic, len = (int)simConfig.len;
	Vector3<Array<FPprecision> > CPUlines, GPUlines;
	Array<electro::pointCharge<FPprecision> > charges(p, 256);
	// Only allocate memory if cpu comparison mode is specified
	if (GPUenable)
		GPUlines.AlignAlloc(n * len);
	if (CPUenable)
		CPUlines.AlignAlloc(n * len);
	perfPacket CPUperf = { 0, 0 }, GPUperf = { 0, 0 };
	std::ofstream data, regress;
	//MainGUI.RegisterProgressIndicator((double * volatile)&CPUperf.progress);

	// Do not activate if memory allocation fails
	if (!CPUlines.GetSize())
		CPUenable = false;
	if (!GPUlines.GetSize())
		GPUenable = false;

	InitializePointChargeArray(charges, p, randseed);

	// init starting points
	Vector3<Array<FPprecision> > *arrMain;
	if (GPUenable) {
		arrMain = &GPUlines;
	} else if (CPUenable) {
		arrMain = &CPUlines;
	} else {
		cerr << " Could not allocate sufficient memory. Halting execution."
		     << endl;
		size_t neededRAM =
			n * len * sizeof(Vector3<FPprecision>) / 1024 / 1024;
		cerr << " " << neededRAM << " MB needed for initial allocation"
		     << endl;
		return 666;
	}

	// Initialize the starting points
	InitializeFieldLineArray(*arrMain, n, nw, nh, nd, randfieldinit);

	// If both CPU and GPU modes are selected, the GPU array will have been
	// initialized first
	// Copy the same starting values to the CPU array
	if (CPUenable && GPUenable)
		CopyFieldLineArray(CPUlines, GPUlines, 0, n);

	// Run calculations
	long long freq, start, end;
	double GPUtime = 0, CPUtime = 0;
	QueryHPCFrequency(&freq);

	if (clMode && CPUenable) {
		//StartConsoleMonitoring ( &CPUperf.progress );
		TestCL(CPUlines, charges, n, 1.0, CPUperf, useCurvature,
		       cl_plat_name);
		CPUperf.progress = 1.0;
		for (size_t i = 0; i < CPUperf.stepTimes.size(); i++) {
			TimingInfo profiler = CPUperf.stepTimes[i];
			cout << profiler.message << ": " << profiler.time
			     << endl;
			if (profiler.bandwidth == 0.0)
				continue;
			cout << "  Bandwidth: " << profiler.bandwidth << " MB/s"
			     << endl;
		}
	} else {
		FPprecision resolution = 1;
		if (GPUenable)
			cout << " GPU" << endl;

		if (CPUenable) {
			StartConsoleMonitoring(&CPUperf.progress);
			QueryHPCTimer(&start);
			CalcField_CPU(CPUlines, charges, n, resolution, CPUperf,
				      useCurvature);
			QueryHPCTimer(&end);
			CPUperf.progress = 1;
			cout << " CPU kernel execution time:\t" << CPUperf.time
			     << " seconds" << endl;
			cout << " Effective performance:\t\t"
			     << CPUperf.performance << " GFLOP/s" << endl;
			CPUtime = double(end - start) / freq;
			cout << " True kernel execution time:\t" << CPUtime
			     << " seconds" << endl;
			if (GPUenable) {
				cout << " Effective speedup:\t\t"
				     << GPUperf.performance /
						CPUperf.performance
				     << " x" << endl;
				cout << " Realistic speedup:\t\t"
				     << CPUtime / GPUtime << " x" << endl;
			}
		}
	}

	FieldRenderer::GLpacket GLdata;
	volatile bool *shouldIQuit = 0;
	if (display) {
		cout << "Initializing display" << endl;
		GLdata.charges =
			(Array<electro::pointCharge<float> > *)&charges;
		GLdata.lines = (Vector3<Array<float> > *)arrMain;
		GLdata.nlines = n;
		GLdata.lineLen = len;
		GLdata.elementSize = sizeof(FPprecision);
		// Before: FieldDisp.RenderPacket(GLdata);
		FieldRenderer::FieldRenderCommData GLmessage;
		GLmessage.messageType = FieldRenderer::SendingGLData;
		GLmessage.commData = (void *)&GLdata;
		FieldDisplay->SendMessage(&GLmessage);

		// Before: FieldDisp.SetPerfGFLOP(GPUperf.performance);
		FieldRenderer::FieldRenderCommData PerfMessage;
		PerfMessage.messageType = FieldRenderer::SendingPerfPointer;
		PerfMessage.commData = (void *)GPUenable ?
						     (&GPUperf.performance) :
						     (&CPUperf.performance);
		FieldDisplay->SendMessage(&PerfMessage);

		// Get ready to quit flag
		FieldRenderer::FieldRenderCommData quitMessage;
		quitMessage.messageType = FieldRenderer::RequestQuitFlag;
		FieldDisplay->SendMessage(&quitMessage);
		shouldIQuit = (bool *)quitMessage.commData;

		try {
			cout << " Starting display" << endl;
			FieldDisplay->StartAsync();
		} catch (char *errString) {
			cerr << " Could not initialize field rendering" << endl;
			cerr << errString << endl;
		}
	} else {
		cout << " Skipping display" << endl;
	}

	// Save points that are significanlty off for regression analysis
	if (regressData && CPUenable && GPUenable)
		compare_electric_fields(CPUlines, GPUlines, n, len,
					"regresion.txt");

	// Wait for renderer to close program if active; otherwise quit directly
	if (display) {
		while (!*shouldIQuit)
			sleep_for(milliseconds(1000));

		FieldDisplay->KillAsync();
	}
	// Tidyness will help in the future
	CPUlines.Free();
	GPUlines.Free();
	charges.Free();
	return 0;
}
