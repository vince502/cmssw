from OxygenHeavyFlavor.Workflow.production_cff import createProcess

process = createProcess(
    ("d", "bToD0K", "bToJpsi", "upsilon"),
    False,
    "HeavyFlavor_OO_DATA.root",
)
