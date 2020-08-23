#! /usr/bin/env bash

function maninstall {
	INST_MAN_DIR="$1"
	MAN_PAGE="$2"

    echo "installing ${MAN_PAGE} in ${INST_MAN_DIR}/man1" 

    [[ "x${INST_MAN_DIR}" == "x" || ! -d "${INST_MAN_DIR}"  ]] && echo "No man directory" && exit 1

	[[ ! -f "${MAN_PAGE}" ]] && echo "Invalid man page specified" && exit 1

	install -D "${MAN_PAGE}" "${INST_MAN_DIR}/man1" 
    echo "done."
}

MAN_FILE="$1"

IFS=":" ; for fl in $(manpath -q); do echo "Install in ${fl}? [y/n]"; read answ; [[ "x${answ}" = "xy" ]] && maninstall "${fl}" "${MAN_FILE}"  && break ; done

exit 0