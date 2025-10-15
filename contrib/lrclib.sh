#!/usr/bin/env sh

TITLE=$1
ARTIST=$2
LENGTH=$3

curl -v --get --data-urlencode="track_name=${TITLE}" --data-urlencode="artist_name=${ARTIST}"  --data-urlencode="track_duration=${TIME}" "https://lrclib.net/api/get" | jq -r .plainLyrics
