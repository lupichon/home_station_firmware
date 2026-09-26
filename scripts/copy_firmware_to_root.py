"""
Copy the firmware binary to the root directory after building.
"""

Import("env")
env.AddPostAction(
    "$BUILD_DIR/firmware.bin",
    env.VerboseAction("cp $BUILD_DIR/firmware.bin firmware.bin", "Copying firmware.bin...")
)