#!/bin/bash

set -x

recording="${8}"
jsonfile="${recording%%.mpg}.json"
destination="/home/media/video/byGenre"

if [ -f "${jsonfile}" ]
then
  eval $( json2shell "${jsonfile}" )

  if [ "${IsMovie}" == 1 ]
  then
    destination="${destination}/Movies"
  else
    destination="${destination}/TV"
  fi

  for genre in "${Genres[@]}"
  do
    case "${genre}" in

    Romance | "Romantic Comedy" | Holiday)
      suffix="Spouse"
      ;;

    Children)
      suffix="Family"
      ;;

    *)
      suffix="${genre}"
      ;;

    esac

    mkdir -p "${destination}/${suffix}/"
    ln "${recording}" "${destination}/${suffix}/"
  done
fi
