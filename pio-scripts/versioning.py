Import("env")

version = env["ENV"].get("GITHUB_REF_NAME")
variant = env["PIOENV"]

if version is None:
    version = "dev"

print("Firmware Version: %s", version)

env.Append(BUILD_FLAGS=['\'-D FIRMWARE_VERSION="%s"\'' % version])
env.Replace(PROGNAME="candle_%s_%s" % (version, variant))