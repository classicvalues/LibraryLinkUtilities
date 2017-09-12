## LibraryLink Utilities

_LibraryLink Utilities_ (abbr. LLU) is a set of modern C++ wrappers for most elements of standard LibraryLink C interface. Containers like MImage and MTensor are wrapped in templated classes. Managing MArguments (both input and output) is also delegated to a separate class:

| LibraryLink element 	|    LLU wrapper   	|
|:-------------------:	|:----------------:	|
| MTensor             	| Tensor<T>        	|
| MRawArray           	| RawArray<T>      	|
| MImage              	| Image<T>         	|
| MArgument           	| MArgumentManager 	|

For more details about each class see [the documentation](http://malgorithmswin.wri.wolfram.com:8080/importexport/LLU).

__The project is new and not really field-tested. Please send all your suggestions and bugs to <rafalc@wolfram.com>__

## Motivation

_LibraryLink_ is a great tool for connecting Wolfram Language with external libraries and programs written in C and it is widely used internally for developing paclets.
But as more and more paclets are now being developed in modern C++ the integration with _LibraryLink_, although still possible, becomes cumbersome and inelegant. Most significant features missing in _LibraryLink_ are:
* Automatic resource management
* Exception handling
* Iterators for MTensor and MRawArray
* Class-like interface for LibraryLink data structures, for example `rank()` as member function of Tensor class instead of separate function 
`mint (*MTensor_getRank)(MTensor)`, or a copy constructor instead of `int (*MTensor_clone)(MTensor, MTensor*)`
* Type safety

The motivation behind _LibraryLink Utilities_ is to provide the aforementioned features without touching _LibraryLink_ sources.

## Code Example

Probably the best way to see how to use LLU and what advantages it has over classic _LibraryLink_ is by comparing the same function written in two different styles. Below we will implement a simple function `repeatCharacters` that takes a string `s` and a tensor `t` and returns a new string `s2` that consists of each character `s[i]` from original string but repeated `t[i]` times, so for example

	repeatCharacters("abc", {3, 2, 1})
gives 

	"aaabbc"  

C - style implementation:
```c

	// global variable which is the buffer for strings returned to LibraryLink
	char* outString = NULL;
	
	EXTERN_C DLLEXPORT int repeatCharacters(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
		char* string = NULL;
		MRawArray counts;
		uint8_t* countsData = NULL;
		size_t outStringIndex = 0;
		size_t len, j;
		mint sum = 0;
		mint c;
		
		string = MArgument_getUTF8String(Args[0]);
		counts = MArgument_getMRawArray(Args[1]);
	
		// check RawArray type
		if (libData->rawarrayLibraryFunctions->MRawArray_getType(counts) != MRawArray_Type_Ubit8) {
			libData->UTF8String_disown(string);
			return LIBRARY_TYPE_ERROR;
		}
	
		// check RawArray rank
		if (libData->rawarrayLibraryFunctions->MRawArray_getRank(counts) != 1) {
			libData->UTF8String_disown(string);
			return LIBRARY_RANK_ERROR;
		}
	
		// check if RawArray length is equal to input string length
		len = strlen(string);
		if (libData->rawarrayLibraryFunctions->MRawArray_getFlattenedLength(counts) != len) {
			libData->UTF8String_disown(string);
			return LIBRARY_DIMENSION_ERROR;
		}
	
		// before we allocate memory for the output string, we have to sum all RawArray elements to see how many bytes are needed
		countsData = (uint8_t*) libData->rawarrayLibraryFunctions->MRawArray_getData(counts);
		for (j = 0; j < len; j++) {
			sum += countsData[j];
		}
	
		// free memory owned by global buffer
		free(outString);
		outString = NULL;
	
		// allocate memory for output string, outString has to be a global variable, because it will be returned to LibraryLink
		outString = (char*) malloc(sum + 1);
		if (!outString) {
			libData->UTF8String_disown(string);
			return LIBRARY_FUNCTION_ERROR;
		}
	
		// populate output string
		for (j = 0; j < len; j++) {
			for (c = 0; c < countsData[j]; c++) {
				outString[outStringIndex++] = string[j];
			}
		}
	
		// add null terminator
		outString[sum] = '\0';
	
		// clean up and set result
		libData->UTF8String_disown(string);
		MArgument_setUTF8String(Res, outString);
	
		return LIBRARY_NO_ERROR;
	}
```

and C++ version with _LibraryLink Utilities_:
```cpp

	EXTERN_C DLLEXPORT int repeatCharactersNew(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res) {
		auto err = LLErrorCode::NoError;
		try {
			// Create manager object
			MArgumentManager mngr(libData, Argc, Args, Res);
	
			// Read string and RawArray arguments
			auto& string = mngr.getString(0);
			auto counts = mngr.getRawArray<std::uint8_t>(1);
	
			// check RawArray rank
			if (counts.rank() != 1) {
				ErrorManager::throwException(LLErrorCode::RankError);
			}
	
			// check if RawArray length is equal to input string length
			if (counts.size() != string.size()) {
				ErrorManager::throwException(LLErrorCode::DimensionsError);
			}
	
			// before we allocate memory for the output string, we have to sum all RawArray elements to see how many bytes are needed
			auto sum = std::accumulate(counts.begin(), counts.end(), static_cast<size_t>(0));
	
			// allocate memory for output string
			std::string outString;
			outString.reserve(sum);
	
			// populate output string
			for (mint i = 0; i < counts.size(); i++) {
				outString.append(std::string(counts[i], string[i]));
			}
	
			// clean up and set result
			mngr.setString(std::move(outString));
		}
		catch (LibraryLinkError& e) {
			err = e.id();
		}
		catch (std::exception&) {
			err = LLErrorCode::FunctionError;
		}
		return err;
	}
```

### Paclets that currently use _LibraryLink Utilities_

- [PacletTemplate](https://stash.wolfram.com/projects/IMEX/repos/paclettemplate) - this is a model paclet for Import/Export developers
- [GIFTools](https://stash.wolfram.com/projects/IMEX/repos/giftools)
- [MediaTools](https://stash.wolfram.com/projects/IMEX/repos/mediatools)
- [HDF5Tools](https://stash.wolfram.com/projects/IMEX/repos/hdf5tools)
- [RAWTools](https://stash.wolfram.com/projects/IMEX/repos/rawtools)

## Limitations with respect to LibraryLink

There are some LibraryLink features currently not covered by _LLU_, most notably:

- Sparse Arrays
- Tensor subsetting: `MTensor_getTensor`
- Raw Array type conversion: `MRawArray_convertType`
- Managed Library Expressions
- Callbacks
- Wolfram IO Library (asynchronous tasks and Data Store)

For now LibraryLink does not allow to write generic code that would clean up memory after Tensors, RawArrays, etc. independently of passing mode used ("Automatic", "Shared", ...). See [this suggestion](http://bugs.wolfram.com/show?number=337331) for more details. In consequence, _LLU_ guarantees to correctly handle only those data structures that were created with _LLU_. Structures received as MArguments will not be automatically freed, therefore you may want to use passing modes that do not require clean-up (like "Constant" or Automatic). In case of "Shared" passing, the only guarantee is that `disown()` will be called on destruction of each object that has positive `shareCount()`. Please consult [LibraryLink tutorial](https://reference.wolfram.com/language/LibraryLink/tutorial/InteractionWithMathematica.html#97446640) for more details.


## How should you incorporate _LibraryLink Utilities_ into your project?

Currently the best way to include LLU into your project is by using git submodule. Submodules are simply git repos inside other repos but working with them may sometimes be tricky. See this excellent [tutorial on submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

For SourceTree users there is also a helpful [blog post](https://blog.sourcetreeapp.com/2012/02/01/using-submodules-and-subrepositories/).

In most cases you will access _LibraryLink Utilities_ in "read-only" manner, i.e. you will just update the submodule to make sure you use the most recent version. 

When you work on your paclet you may occasionally find a bug in LLU or a missing feature. You should either report it or try to make changes yourself following the usual workflow: create new branch - implement changes - open PR - merge. It is possible to do it with LLU as submodule in your project but you should really read the tutorial before you try.

### Quick guide for Import/Export developers
Here is a list of commands that will be useful to developers working on Import/Export paclets (names ending with "Tools"). Usually these paclets have _CPPSource/_ directory containing the source code. It is easy to modify these commands so that they work for arbitrary project.

#### Adding _LibraryLink Utilities_ to your paclet
`git submodule add ssh://git@stash.wolfram.com:7999/imex/librarylinkutilities.git CPPSource/LibraryLinkUtilities`

#### Cloning a project that already uses _LibraryLink Utilities_
`git clone --recursive ssh://git@stash.wolfram.com:7999/imex/<paclet_name_in_lowercase>.git`

#### Updating _LibraryLink Utilities_ in your project
`git submodule update --remote CPPSource/LibraryLinkUtilities/`


### Compilation
After checking out the submodule remember to modify your build script accordingly so that _LLU_ sources also get compiled. Since the source code uses C++14 features, you have to make sure you enabled C++14 support in your compiler. *Visual Studio 2015* or later provides the support by default and in *gcc* or *clang* you may have to add **-std=c++14** flag. 

Minimum required version of *gcc* is 5 and for *clang* it is 3.4.

## API Reference

Doxygen is used to generate documentation for _LibraryLink Utilities_ API. You can browse generated docs online here: 

<http://malgorithmswin.wri.wolfram.com:8080/importexport/LLU>


## Contributors

* Rafał Chojna (<rafalc@wolfram.com>) - main developer
* Sean Cheren  (<scheren@wolfram.com>) - top-level code for error handling