/**
 * @file	ErrorManager.cpp
 * @author	Rafal Chojna <rafalc@wolfram.com>
 * @date	March 21, 2019
 * @brief	
 */
#include "LLU/ErrorLog/ErrorManager.h"

#include "LLU/LibraryData.h"
#include "LLU/Utilities.hpp"
#include "LLU/ML/MLStream.hpp"
#include "LLU/ML/Utilities.h"

namespace LLU {

	auto ErrorManager::errors() -> ErrorManager::ErrorMap& {
		static ErrorMap errMap = registerLLUErrors({
			// Original LibraryLink error codes:
			{ErrorName::VersionError,                "An error was caused by an incompatible function call. The library was compiled with a previous LibraryData version." },
			{ErrorName::FunctionError,               "An error occurred in the library function." },
			{ErrorName::MemoryError,                 "An error was caused by failed memory allocation or insufficient memory." },
			{ErrorName::NumericalError,              "A numerical error was encountered." },
			{ErrorName::DimensionsError,             "An error caused by inconsistent dimensions or by exceeding array bounds." },
			{ErrorName::RankError,                   "An error was caused by a tensor with an inconsistent rank." },
			{ErrorName::TypeError,                   "An error caused by inconsistent types was encountered." },
			{ErrorName::NoError,                     "No errors occurred." },

			// LibraryData errors:
			{ErrorName::LibDataError,                "WolframLibraryData is not set. Make sure to call LibraryData::setLibraryData in WolframLibrary_initialize."},

			// MArgument errors:
			{ErrorName::MArgumentIndexError,         "An error was caused by an incorrect argument index." },
			{ErrorName::MArgumentNumericArrayError,  "An error was caused by a NumericArray argument." },
			{ErrorName::MArgumentTensorError,        "An error was caused by a Tensor argument." },
			{ErrorName::MArgumentImageError,         "An error was caused by an Image argument." },

			// ErrorManager errors:
			{ErrorName::ErrorManagerThrowIdError,    "An exception was thrown with a non-existent id." },
			{ErrorName::ErrorManagerThrowNameError,  "An exception was thrown with a non-existent name." },
			{ErrorName::ErrorManagerCreateNameError, "An exception was registered with a name that already exists." },

			// NumericArray errors:
			{ErrorName::NumericArrayNewError,        "Failed to create a new NumericArray." },
			{ErrorName::NumericArrayCloneError,      "Failed to clone NumericArray." },
			{ErrorName::NumericArrayTypeError,       "An error was caused by an NumericArray type mismatch." },
			{ErrorName::NumericArraySizeError,       "An error was caused by an incorrect NumericArray size." },
			{ErrorName::NumericArrayIndexError,      "An error was caused by attempting to access a nonexistent NumericArray element." },
			{ErrorName::NumericArrayConversionError, "Failed to convert NumericArray from different type."},

			// MTensor errors:
			{ErrorName::TensorNewError,              "Failed to create a new MTensor." },
			{ErrorName::TensorCloneError,            "Failed to clone MTensor." },
			{ErrorName::TensorTypeError,             "An error was caused by an MTensor type mismatch." },
			{ErrorName::TensorSizeError,             "An error was caused by an incorrect Tensor size." },
			{ErrorName::TensorIndexError,            "An error was caused by attempting to access a nonexistent Tensor element." },

			// MImage errors:
			{ErrorName::ImageNewError,               "Failed to create a new MImage." },
			{ErrorName::ImageCloneError,             "Failed to clone MImage." },
			{ErrorName::ImageTypeError,              "An error was caused by an MImage type mismatch." },
			{ErrorName::ImageSizeError,              "An error was caused by an incorrect Image size." },
			{ErrorName::ImageIndexError,             "An error was caused by attempting to access a nonexistent Image element." },

			// General container errors:
			{ErrorName::CreateFromNullError,         "Attempting to create a generic container from nullptr."},
			{ErrorName::MArrayElementIndexError,     "Attempting to access MArray element at invalid index."},
			{ErrorName::MArrayElementIndexError,     "Attempting to access MArray dimension `d` which does not exist."},

			// MathLink errors:
			{ErrorName::MLNullMlinkError,        "Trying to create MLStream with NULL MLINK"},
			{ErrorName::MLTestHeadError,             "MLTestHead failed (wrong head or number of arguments)." },
			{ErrorName::MLPutSymbolError,            "MLPutSymbol failed." },
			{ErrorName::MLPutFunctionError,          "MLPutFunction failed." },
			{ErrorName::MLTestSymbolError,           "MLTestSymbol failed (different symbol on the link than expected)." },
			{ErrorName::MLWrongSymbolForBool,        R"(Tried to read something else than "True" or "False" as boolean.)" },
			{ErrorName::MLGetListError,              "Could not get list from MathLink." },
			{ErrorName::MLGetScalarError,            "Could not get scalar from MathLink." },
			{ErrorName::MLGetStringError,            "Could not get string from MathLink." },
			{ErrorName::MLGetArrayError,             "Could not get array from MathLink." },
			{ErrorName::MLPutListError,              "Could not send list via MathLink." },
			{ErrorName::MLPutScalarError,            "Could not send scalar via MathLink." },
			{ErrorName::MLPutStringError,            "Could not send string via MathLink." },
			{ErrorName::MLPutArrayError,             "Could not send array via MathLink." },
			{ErrorName::MLGetSymbolError,            "MLGetSymbol failed." },
			{ErrorName::MLGetFunctionError,          "MLGetFunction failed." },
			{ErrorName::MLPacketHandleError,         "One of the packet handling functions failed." },
			{ErrorName::MLFlowControlError,          "One of the flow control functions failed." },
			{ErrorName::MLTransferToLoopbackError,   "Something went wrong when transferring expressions from loopback link." },
			{ErrorName::MLCreateLoopbackError,       "Could not create a new loopback link." },
			{ErrorName::MLLoopbackStackSizeError,    "Loopback stack size too small to perform desired action." },

			// DataList errors:
			{ErrorName::DLNullRawNode,               "DataStoreNode passed to Node wrapper was null" },
			{ErrorName::DLInvalidNodeType,           "DataStoreNode passed to Node wrapper carries data of invalid type" },
			{ErrorName::DLGetNodeDataError,          "DataStoreNode_getData failed" },
			{ErrorName::DLNullRawDataStore,          "DataStore passed to DataList was null" },
			{ErrorName::DLPushBackTypeError,         "Element to be added to the DataList has incorrect type" },

			// MArgument errors:
			{ErrorName::ArgumentCreateNull,          "Trying to create Argument object from nullptr" },
			{ErrorName::ArgumentAddNodeMArgument,    "Trying to add DataStore Node of type MArgument (aka MType_Undef)" },

			// ProgressMonitor errors:
			{ErrorName::Aborted,                     "Computation aborted by the user." },

			// ManagedExpression errors:
			{ErrorName::ManagedExprInvalidID,        "Given number is not an ID of any existing managed expression." },
			{ErrorName::MLEDynamicTypeError,         "Invalid dynamic type requested for a Managed Library Expression" },
		});
		return errMap;
	}

	bool ErrorManager::sendParametersImmediately = true;

	int& ErrorManager::nextErrorId() {
		static int id = ErrorCode::VersionError;
		return id;
	}

	auto ErrorManager::registerLLUErrors(std::initializer_list<ErrorStringData> initList) -> ErrorMap {
		ErrorMap e;
		for (auto&& err : initList) {
			e.emplace(err.first, LibraryLinkError { nextErrorId()--, err.first, err.second });
		}
		return e;
	}

	void ErrorManager::registerPacletErrors(const std::vector<ErrorStringData>& errs) {
		for (auto&& err : errs) {
			set(err);
		}
	}

	void ErrorManager::set(const ErrorStringData& errorData) {
		auto& errorMap = errors();
		auto elem = errorMap.emplace(errorData.first, LibraryLinkError { nextErrorId()--, errorData.first, errorData.second });
		if (!elem.second) {
			// Revert nextErrorId because nothing was inserted
			nextErrorId()++;

			// Throw only if someone attempted to insert an error with existing key but different message
			if (elem.first->second.message() != errorData.second) {
				throw errors().find("ErrorManagerCreateNameError")->second;
			}
		}
	}

	const LibraryLinkError& ErrorManager::findError(int errorId) {
		for (auto&& err : errors()) {
			if (err.second.id() == errorId) {
				return err.second;
			}
		}
		throw errors().find("ErrorManagerThrowIdError")->second;
	}

	const LibraryLinkError& ErrorManager::findError(const std::string& errorName) {
		const auto& exception = errors().find(errorName);
		if (exception == errors().end()) {
			throw errors().find("ErrorManagerThrowNameError")->second;
		}
		return exception->second;
	}

	void ErrorManager::sendRegisteredErrorsViaMathlink(MLINK mlp) {
		MLStream<ML::Encoding::UTF8> ms(mlp, "List", 0);

		ms << ML::NewPacket << ML::Association(static_cast<int>(errors().size()));

		for (const auto& err : errors()) {
			ms << ML::Rule << err.first << ML::List(2) << err.second.id() << err.second.message();
		}

		ms << ML::EndPacket << ML::Flush;
	}

	EXTERN_C DLLEXPORT int sendRegisteredErrors(WolframLibraryData libData, MLINK mlp) {
		Unused(libData);
		auto err = ErrorCode::NoError;
		try {
			ErrorManager::sendRegisteredErrorsViaMathlink(mlp);
		}
		catch (LibraryLinkError& e) {
			err = e.which();
		}
		catch (...) {
			err = ErrorCode::FunctionError;
		}
		return err;
	}
}
