import os

Import("env")
env['PROJECT_SRC_DIR'] = env['PROJECT_DIR'] + os.sep + "examples" + os.sep + env["PIOENV"]
print("Setting the project directory to: {}".format(env['PROJECT_SRC_DIR']))
