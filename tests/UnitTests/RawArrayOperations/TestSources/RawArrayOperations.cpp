#include "WolframLibrary.h"
#include "WolframRawArrayLibrary.h"

#include "MArgumentManager.h"
#include "LibraryLinkFunctionMacro.h"

using namespace LibraryLinkUtils;

static MRawArray shared_raw = 0;

EXTERN_C DLLEXPORT mint WolframLibrary_getVersion() {
	return WolframLibraryVersion;
}

EXTERN_C DLLEXPORT int WolframLibrary_initialize(WolframLibraryData libData) {
	MArgumentManager::setLibraryData(libData);
	return 0;
}

LIBRARY_LINK_FUNCTION(echoRawArray) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnRawArray(0, [&mngr](auto&& rarray1) {
			auto rarray2(std::move(rarray1));
			mngr.setRawArray(rarray2);
		});
	}
	catch (LibraryLinkError<LLErrorCode>& e) {
		err = e.which();
		std::cout << e.what() << std::endl;
	}
	catch (std::exception& e) {
		err = LLErrorCode::FunctionError;
	}
	return static_cast<int>(err);
}

/*
 * Raw array library functions
 */
LIBRARY_LINK_FUNCTION(getRawArrayLength) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnRawArray(0, [&mngr](auto&& rarray) {
			mngr.setInteger(rarray.size());
		});
	}
	catch (LibraryLinkError<LLErrorCode>& e) {
		err = e.which();
	}
	catch (std::exception& e) {
		err = LLErrorCode::FunctionError;
	}
	return static_cast<int>(err);
}

LIBRARY_LINK_FUNCTION(getRawArrayRank) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnRawArray(0, [&mngr](auto&& rarray) {
			mngr.setInteger(rarray.rank());
		});
	}
	catch (LibraryLinkError<LLErrorCode>& e) {
		err = e.which();
	}
	catch (std::exception& e) {
		err = LLErrorCode::FunctionError;
	}
	return static_cast<int>(err);
}

//create new raw array
LIBRARY_LINK_FUNCTION(newRawArray) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		RawArray<float> ra(0., { 3, 3 });
		mngr.setRawArray(ra);
	}
	catch (LibraryLinkError<LLErrorCode>& e) {
		err = e.which();
	}
	catch (std::exception& e) {
		err = LLErrorCode::FunctionError;
	}
	return static_cast<int>(err);
}

//clone RawArray
LIBRARY_LINK_FUNCTION(cloneRawArray) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnRawArray(0, [&mngr](auto&& rarray1) {
			auto rarray2 = rarray1;
			mngr.setRawArray(rarray2);
		});
	}
	catch (LibraryLinkError<LLErrorCode>& e) {
		err = e.which();
	}
	catch (std::exception& e) {
		err = LLErrorCode::FunctionError;
	}
	return static_cast<int>(err);
}

LIBRARY_LINK_FUNCTION(changeSharedRawArray) {
	WolframRawArrayLibrary_Functions rawArrayFunctions = libData->rawarrayLibraryFunctions;
	int err = LIBRARY_NO_ERROR;

	if (shared_raw) {
		rawArrayFunctions->MRawArray_disown(shared_raw);
		shared_raw = 0;
	}

	shared_raw = MArgument_getMRawArray(Args[0]);

	return err;
}

EXTERN_C DLLEXPORT int getSharedRawArray(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
	int err = LIBRARY_NO_ERROR;

	if (shared_raw) {
		MArgument_setMRawArray(Res, shared_raw);
	}
	else
		err = LIBRARY_FUNCTION_ERROR;

	return err;
}

struct ZeroReal64 {
	template<typename T>
	void operator()(RawArray<T>, MArgumentManager&) {
		throw LibraryLinkError<LLErrorCode>(LLErrorCode::FunctionError);
	}

	void operator()(RawArray<double>& ra, MArgumentManager& mngr) {
		std::fill(ra.begin(), ra.end(), 0.0);
		mngr.setRawArray(ra);
	}
};

//reset rawArray
EXTERN_C DLLEXPORT int rawZeroData(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
	auto err = LLErrorCode::NoError;
	try {
		MArgumentManager mngr(Argc, Args, Res);
		mngr.operateOnRawArray<ZeroReal64>(0, mngr);
	}
	catch (LibraryLinkError<LLErrorCode>& e) {
		err = e.which();
	}
	catch (std::exception& e) {
		err = LLErrorCode::FunctionError;
	}
	return static_cast<int>(err);
}
