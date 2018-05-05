for movie in `find . -name *.mov`; do
  echo convert  $movie to ${movie%.mov}.mp4
  ffmpeg -i $movie -c:v h264 -preset ultrafast -tune zerolatency -crf 17 -c:a copy ${movie%.mov}.mp4
done
# avconv -i $1  -threads auto -qscale 1 -aq 1 -vcodec mjpeg $2
# avconv -i $1 ${2%.avi}.wav
