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
#ifndef _CL_ELECTROSTATICS_HPP
#define _CL_ELECTROSTATICS_HPP

#include "ElectrostaticFunctor.hpp"
#include "Electrostatics.h"
#include "CL_Manager.hpp"

typedef int CLerror;

/**=============================================================================
 * \ingroup DEVICE_FUNCTORS
 * @{
 * ===========================================================================*/
template <class T> class CLElectrosFunctor : public ElectrostaticFunctor<T> {
public:
	CLElectrosFunctor();
	~CLElectrosFunctor();

	/*-----------------------AbstractFunctor overriders---------------------
	 * These functions implement the pure functions specified by
	 * AbstractFunctor. They can be called externally, and will attach and
	 * detach the device context accordingly. These functions cannot be
	 *.considered thread safe
	 * The sequential order is that of AbstractFunctor:
	 * BindData()
	 * AllocateResources()
	 * Run() - this calls the main functor
	 * Executing these functions simultaneously or in a different order will
	 * cause them to fail
	 */
	void BindData(void *dataParameters);
	void AllocateResources();
	void ReleaseResources();
	unsigned long MainFunctor(size_t functorIndex, size_t deviceIndex);
	unsigned long AuxFunctor();
	void PostRun();
	bool Fail();
	bool FailOnFunctor(size_t functorIndex);

	void GenerateParameterList(size_t *nDevices);
	void SetPreferredPlatform(const char *partial_platform_name)
	{
		m_preferred_platform = std::string(partial_platform_name);
	}

private:
	/// Specifies the error code incurred during the last global operation
	CLerror m_lastOpErrCode;

	/// Records the total number of available compute devices
	size_t m_nDevices;

	static OpenCL::ClManager m_DeviceManager;

	/// Partitions the Data for different devices
	void PartitionData();

	// Device and functor  related information
	class FunctorData {
	public:
		/// ID of the device this functor is intended to run on
		OpenCL::ClManager::clDeviceProp *device;
		/// Context associated with the device
		cl_context context;
		/// Device buffer for the field line
		Vector3<cl_mem> devFieldMem;
		/// Device buffer for point charges
		cl_mem chargeMem;
		/// X-size of kernel block, dependent on selected kernel (MT/NON_MT)
		unsigned int blockXSize;
		/// kernel block size, dependent on selected kernel (MT/NON_MT)
		unsigned int blockDim;
		/// Number of lines to be processed by this device
		size_t widthGlobX;
		/// Number of blocks that can be launched during a kernel call
		size_t nBlocksPerSegment;
		/// Keeps track of errors that ocuur on the context current to the
		/// functor
		CLerror lastOpErrCode;

		/// Functor specific data

		/// The starting index of pFieldLinesData that has been assigned to this
		/// functor
		size_t startIndex;
		/// The number of field lines from 'startIndex' that has been assigned
		/// to this functor
		size_t elements;
		/// Number of steps
		size_t steps;
		/// The preffered vector width of the device
		size_t vecWidth;
		/// Global and local work-sizes
		size_t global[3], local[3];
		/// The OpenCL kernel
		cl_kernel kernel;
		/// the command queue used for execution
		cl_command_queue queue;
		/// Functor-specific performance information
		perfPacket perfData;
	};
	/// Contains data for each individual functor
	std::vector<FunctorData> m_functors;
	std::string m_preferred_platform;

	CLerror LoadKernels(size_t deviceID);

	static size_t FindVectorWidth(OpenCL::ClManager::clDeviceProp &dev);
	static const char *FindPrecType();
	static const char *FindVecType(size_t vecWidth);
};
///@}
extern CLElectrosFunctor<float> CLtest;

/**=============================================================================
 * \brief Electrostatics functor constructor
 *
 * Initializes critical variables
 * ===========================================================================*/
template <class T> CLElectrosFunctor<T>::CLElectrosFunctor() : m_nDevices(0)
{
}

#endif //CL_ELECTROSTATICS_HPP
