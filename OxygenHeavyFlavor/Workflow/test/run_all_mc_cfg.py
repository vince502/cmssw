from OxygenHeavyFlavor.Workflow.production_cff import createProcess

process = createProcess(
    ("d", "bToD0K", "bToJpsi", "upsilon"),
    True,
    "HeavyFlavor_OO_MC.root",
)
