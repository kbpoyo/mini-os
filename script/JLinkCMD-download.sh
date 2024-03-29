
JLINK_HOME=/mnt/f/software/common_tools/Jlink/JLink
# JLink_CMD_ARG=" -autoconnect 1 -device S3C2440A -speed 4000 -if jtag -jtagconf -1,-1 -commandfile ./download.jlink"
JLink_CMD_ARG="-commandfile ./download.jlink"

$JLINK_HOME/JLink.exe $JLink_CMD_ARG