    
    let progress = document.getElementById("progress");
    let song = document.getElementById("song");
    let ctrlIcon = document.getElementById("ctrlIcon");
    let currentTime = document.getElementById("currentTime");
    let duration = document.getElementById("duration");
    let songImg = document.getElementById("foto");

    song.onloadedmetadata = function() {
      progress.max = song.duration;
      progress.value = song.currentTime;
      duration.textContent = formatTime(song.duration);

    }

    function playPause(){
      if(ctrlIcon.classList.contains("fa-pause")){
        song.pause();
        ctrlIcon.classList.remove("fa-pause")
        ctrlIcon.classList.add("fa-play")
        songImg.classList.remove("rotate")
      }else{
        song.play();
        ctrlIcon.classList.add("fa-pause")
        ctrlIcon.classList.remove("fa-play")
        songImg.classList.add("rotate")
      }
    }

    if(song.play()){
      setInterval(() => {
        progress.value = song.currentTime
      }, 500);
    }

    progress.onchange = function() {
      song.play();
      song.currentTime = progress.value
      ctrlIcon.classList.add("fa-pause")
      ctrlIcon.classList.remove("fa-play")
    }

    song.addEventListener("timeupdate", function() {
      progress.value = song.currentTime;
      currentTime.textContent = formatTime(song.currentTime);
    });

    progress.oninput = function() {
      song.currentTime = progress.value;
    }

    function formatTime(time) {
      const minutes = Math.floor(time / 60);
      const seconds = Math.floor(time % 60);
      return `${minutes}:${seconds.toString().padStart(2, '0')}`;
    }
