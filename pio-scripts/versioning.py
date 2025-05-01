Import("env")

version = env["ENV"].get("FIRMWARE_VERSION")

if version is None:
    version = "0.0.0-dev"

env.Append(BUILD_FLAGS=['\'-D FIRMWARE_VERSION="%s"\'' % version])