(* Project base directory *)
$baseDir = FileNameDrop[$CurrentFile, -3];

(* Path to directory containing include folder from LibraryLinkUtilities installation *)
$LLUIncDir = FileNameJoin[{$baseDir, "install", "include"}];

(* Path to LibraryLinkUtilities static lib *)
$LLULibDir = FileNameJoin[{$baseDir, "install", "lib"}];

(* LibraryLinkUtilities library name *)
$LLULib = "LLU";

(* Path to LibraryLinkUtilities shared resources *)
$LLUSharedDir = FileNameJoin[{$baseDir, "install", "share"}];

(* The majority of unit tests should compile with C++14. For the rest we add a global config variable to modify. *)
$CppVersion = "c++14";

(* Compilations options for all tests *)
options := {
	"CleanIntermediate" -> True,
	"IncludeDirectories" -> { $LLUIncDir },
	"Libraries" -> { $LLULib },
	"LibraryDirectories" -> { $LLULibDir },
	"CompileOptions" ->
		If[MatchQ[$SystemID, "Windows-x86-64" | "Windows"],
			"/O2 /EHsc /W3 /std:" <> $CppVersion
			,
			"-O2 -Wall --pedantic -fvisibility=hidden -std=" <> $CppVersion
		],
	"ShellOutputFunction" -> Print,
	"ShellCommandFunction" -> Print,
	"Language" -> "C++",
	"TransferProtocolLibrary" -> "WSTP"
};

(* If dynamic version of LLU was built, we want to load it to Mathematica before test libs are loaded *)
LibraryLoad /@ FileNames[{"*.so", "*.dll", "*.dylib"}, $LLULibDir];


(* Helper definitions *)

TopLevelErrorCodeQ[c_Integer] := c > 7;
TopLevelErrorCodeQ[_] := False;

LLErrorCodeQ[c_Integer] := 0 <= c <= 7;
LLErrorCodeQ[_] := False;

CppErrorCodeQ[c_Integer] := c < 0;
CppErrorCodeQ[_] := False;

LoggerStringTest = (AllTrue[MapThread[StringEndsQ, {##}], TrueQ]&);

(* Memory leak test *)
ClearAll[MemoryLeakTest];
SetAttributes[MemoryLeakTest, HoldAll];
Options[MemoryLeakTest] = {"ReturnValue" -> Last};

MemoryLeakTest[expression_, opts : OptionsPattern[]] :=
	MemoryLeakTest[expression, {i, 10}, opts];

MemoryLeakTest[expression_, repetitions_Integer?Positive, opts : OptionsPattern[]] :=
	MemoryLeakTest[expression, {i, repetitions}, opts];

MemoryLeakTest[expression_, {s_Symbol, repetitions__}, opts : OptionsPattern[]] :=
	Block[{$MessageList},
		Module[{res, memory},
			$MessageList = {};
			ClearSystemCache[];
			res = Table[
				memory = MemoryInUse[];
				expression;
				$MessageList = {};
				ClearSystemCache[];
				MemoryInUse[] - memory
				,
				{s, repetitions}
			];
			OptionValue["ReturnValue"] @ res
		]
	];

MemoryLeakTestWithMessages[expression_] :=
	MemoryLeakTestWithMessages[expression, 10];

MemoryLeakTestWithMessages[expression_, repetitions_Integer?Positive] :=
	Block[{mem},
		Do[
			mem = MemoryInUse[];
			expression;
			Print[MemoryInUse[] - mem]
			,
			{repetitions}
		]
	];