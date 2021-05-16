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
#ifndef _CL_MANAGER_H
#define _CL_MANAGER_H

#define CL_TARGET_OPENCL_VERSION 110
#include <CL/cl.h>
#include <iostream>
#include <vector>

/**=============================================================================
 * \defgroup DEVICE_MANAGERS Device Managers
 *
 * @{
 * ===========================================================================*/
namespace deviceMan
{
	class ComputeDeviceManager {
	public:
		ComputeDeviceManager();
		virtual ~ComputeDeviceManager(){};

		/// Returns the total number of Compute devices detected
		virtual size_t GetNumDevices() = 0;

	protected:
		//-------------------Global Context tracking------------------//
		/// Has a scan for compatible devices already been completed ?
		static bool deviceScanComplete;

		/// Performs a one-time scan to determine the number of devices
		/// and obtain the device properties
		virtual void ScanDevices() = 0;
	};
} // namespace deviceMan
/**=============================================================================
 * @}
 * ===========================================================================*/

namespace OpenCL
{
using std::vector;

class ClManager : public deviceMan::ComputeDeviceManager {
public:
	/// Keeps track of the properties of a device
	class clDeviceProp {
	public:
		clDeviceProp(cl_device_id device);
		~clDeviceProp();

	public:
		/** \brief The OpenCL ID of the platform*/
		cl_device_id deviceID;
		/** \brief CL_DEVICE_ADDRESS_BITS*/
		cl_uint addressBits;
		/** \brief CL_DEVICE_AVAILABLE*/
		cl_bool available;
		/** \brief CL_DEVICE_COMPILER_AVAILABLE*/
		cl_bool compilerAvailable;
		/** \brief CL_DEVICE_DOUBLE_FP_CONFIG*/
		cl_device_fp_config doubleFpConfig;
		/** \brief CL_DEVICE_ENDIAN_LITTLE*/
		cl_bool littleEndian;
		/** \brief CL_DEVICE_ERROR_CORRECTION_SUPPORT*/
		cl_bool EccSupport;
		/** \brief CL_DEVICE_EXECUTION_CAPABILITIES*/
		cl_device_exec_capabilities execCapabilities;
		/** \brief CL_DEVICE_EXTENSIONS */
		char extensions[1024];

		/** \brief CL_DEVICE_GLOBAL_MEM_CACHE_SIZE*/
		cl_ulong globalMemCacheSize;
		/** \brief CL_DEVICE_GLOBAL_MEM_CACHE_TYPE*/
		cl_device_mem_cache_type memCacheType;
		/** \brief CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE*/
		cl_uint globalMemCachelineSize;
		/** \brief CL_DEVICE_GLOBAL_MEM_SIZE*/
		cl_ulong globalMemSize;

		/** \brief CL_DEVICE_HALF_FP_CONFIG*/
		cl_device_fp_config halfFpConfig;

		/** \brief CL_DEVICE_IMAGE_SUPPORT */
		cl_bool imageSupport;
		/** \brief CL_DEVICE_IMAGE2D_MAX_HEIGHT */
		size_t image2DMaxHeight;
		/** \brief CL_DEVICE_IMAGE2D_MAX_WIDTH */
		size_t image2DMaxWidth;
		/** \brief CL_DEVICE_IMAGE3D_MAX_DEPTH */
		size_t image3DMaxDepth;
		/** \brief CL_DEVICE_IMAGE3D_MAX_HEIGHT */
		size_t image3DMaxHeight;
		/** \brief CL_DEVICE_IMAGE3D_MAX_WIDTH */
		size_t image3DMaxWidth;

		/** \brief CL_DEVICE_LOCAL_MEM_SIZE*/
		cl_ulong localMemSize;
		/** \brief CL_DEVICE_LOCAL_MEM_TYPE*/
		cl_device_local_mem_type localMemType;

		/** \brief CL_DEVICE_MAX_CLOCK_FREQUENCY*/
		cl_uint maxClockFrequency;
		/** \brief CL_DEVICE_MAX_COMPUTE_UNITS*/
		cl_uint maxComputeUnits;
		/** \brief CL_DEVICE_MAX_CONSTANT_ARGS*/
		cl_uint maxConstantArgs;
		/** \brief CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE*/
		cl_ulong maxConstantBufferSize;
		/** \brief CL_DEVICE_MAX_MEM_ALLOC_SIZE*/
		cl_ulong maxMemAllocSize;
		/** \brief CL_DEVICE_MAX_PARAMETER_SIZE */
		size_t maxParameterSize;
		/** \brief CL_DEVICE_MAX_READ_IMAGE_ARGS*/
		cl_uint maxReadImageArgs;
		/** \brief CL_DEVICE_MAX_SAMPLERS */
		cl_uint maxSamplers;
		/** \brief CL_DEVICE_MAX_WORK_GROUP_SIZE */
		size_t maxWorkGroupSize;
		/** \brief CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS*/
		cl_uint maxWorkItemDimensions;
		/** \brief CL_DEVICE_MAX_WORK_ITEM_SIZES */
		size_t *maxWorkItemSizes;
		/** \brief CL_DEVICE_MAX_WRITE_IMAGE_ARGS */
		cl_uint maxWriteImageArgs;

		/** \brief CL_DEVICE_MEM_BASE_ADDR_ALIGN */
		cl_uint memBaseAddrAlign;
		/** \brief CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE */
		cl_uint minDataTypeAlignSize;

		/** \brief CL_DEVICE_NAME */
		char name[256];
		/** \brief CL_DEVICE_PLATFORM*/
		cl_platform_id platform;

		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR*/
		cl_uint preferredVectorWidth_char;
		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT*/
		cl_uint preferredVectorWidth_short;
		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT*/
		cl_uint preferredVectorWidth_int;
		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG */
		cl_uint preferredVectorWidth_long;
		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT */
		cl_uint preferredVectorWidth_float;
		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE */
		cl_uint preferredVectorWidth_double;
		/** \brief CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF */
		cl_uint preferredVectorWidth_half;

		/** \brief CL_DEVICE_HOST_UNIFIED_MEMORY */
		cl_bool hostUnifiedMemory;

		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR */
		cl_uint nativeVectorWidth_char;
		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT */
		cl_uint nativeVectorWidth_short;
		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_INT */
		cl_uint nativeVectorWidth_int;
		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG */
		cl_uint nativeVectorWidth_long;
		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT */
		cl_uint nativeVectorWidth_float;
		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE */
		cl_uint nativeVectorWidth_double;
		/** \brief CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF */
		cl_uint nativeVectorWidth_half;

		/** \brief CL_DEVICE_OPENCL_C_VERSION */
		char openCL_C_version[256];
		/** \brief CL_DEVICE_PROFILE */
		char deviceProfile[256];

		/** \brief CL_DEVICE_PROFILING_TIMER_RESOLUTION */
		size_t profilingTimerResolution;

		/** \brief CL_DEVICE_QUEUE_PROPERTIES */
		cl_command_queue_properties queueProperties;

		/** \brief CL_DEVICE_SINGLE_FP_CONFIG */
		cl_device_fp_config singleFpConfig;

		/** \brief CL_DEVICE_TYPE */
		cl_device_type type;
		/** \brief CL_DEVICE_VENDOR */
		char vendor[256];
		/** \brief CL_DEVICE_VENDOR_ID */
		cl_uint vendorID;
		/** \brief CL_DEVICE_VERSION */
		char deviceVersion[256];
		/** \brief CL_DRIVER_VERSION */
		char driverVersion[256];

		/// Fills the properties with those of the chosen 'deviceID'
		void SetDeviceID(cl_device_id deviceID);
	};

	/// Keeps track of the properties of a platform and all its devices
	class clPlatformProp {
	public:
		/**
		 * Fills the properties with those of the corresponding
		 * platformID, and all its devices
		 */
		clPlatformProp(cl_platform_id platformID);
		~clPlatformProp();

	public:
		///\brief The OpenCL ID of the platform
		cl_platform_id platformID;

		/** \brief CL_PLATFORM_PROFILE */
		char profile[256];

		/** \brief CL_PLATFORM_VERSION */
		char version[256];

		/** \brief CL_PLATFORM_NAME */
		char name[256];

		/** \brief CL_PLATFORM_VENDOR */
		char vendor[256];

		/** \brief CL_PLATFORM_EXTENSIONS */
		char extensions[1024];

		/** \brief List of all devices in the platform */
		vector<clDeviceProp *> devices;
	};

	ClManager();
	~ClManager();

	/**
	 * Lists all devices that were found during the last scan, including
	 * non-compatible ones
	 */
	static void ListAllDevices(std::ostream &out = std::cout);

	size_t GetNumDevices();

private:
	//------------------------Global Context tracking---------------------//
	/** List of all platforms found on the machine
	 * It seems that due to a bug in GCC, a static std::vector may fall out
	 * of scope. Just use a pointer for the time being, allocate it with
	 * new, and it won't ever fall out of scope
	 */
	static vector<clPlatformProp *> *platforms;

	/// DeviceManager overriders
	///{@
	void ScanDevices();

public:
	vector<clPlatformProp *> &fstGetPlats()
	{
		return *platforms;
	};

	///}@
};

extern ClManager GlobalClManager;

} // Namespace OpenCL

#endif //_CL_MANAGER_H
