import os
import unreal


OUT = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "InspectUnrealPython.txt")


def log(name, value):
    line = "[InspectUnrealPython] " + name + ": " + str(value)
    unreal.log(line)
    with open(OUT, "a", encoding="utf-8") as handle:
        handle.write(line + "\n")


with open(OUT, "w", encoding="utf-8") as handle:
    handle.write("")


names = dir(unreal)
keywords = [
    "Blueprint",
    "K2",
    "Graph",
    "Node",
    "Subobject",
    "Component",
    "Editor",
    "SimpleConstruction",
    "SCS",
]

for keyword in keywords:
    matches = [name for name in names if keyword.lower() in name.lower()]
    log(keyword, matches[:120])

for name in [
    "BlueprintEditorLibrary",
    "BlueprintEditorSubsystem",
    "SubobjectDataSubsystem",
    "SubobjectDataBlueprintFunctionLibrary",
    "K2Node_CallFunction",
    "K2Node_Event",
    "K2Node_InputKey",
    "EdGraph",
    "EdGraphSchema_K2",
    "AddNewSubobjectParams",
]:
    log(name, hasattr(unreal, name))

for name in [
    "SubobjectDataSubsystem",
    "SubobjectDataBlueprintFunctionLibrary",
    "AddNewSubobjectParams",
    "BlueprintEditorLibrary",
    "EdGraph",
    "EdGraphNode",
    "K2Node",
    "K2Node_CallFunction",
]:
    obj = getattr(unreal, name, None)
    if obj:
        log(name + ".dir", dir(obj)[:200])

log("OUT", OUT)
