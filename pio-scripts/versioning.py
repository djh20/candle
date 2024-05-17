Import("env")

version = env["ENV"].get("FIRMWARE_VERSION")

if version is None:
    version = "dev"

env.Append(BUILD_FLAGS=['\'-D FIRMWARE_VERSION="%s"\'' % version])