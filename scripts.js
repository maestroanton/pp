    let progress = document.getElementById("progress");
    let song = document.getElementById("song");
    let ctrlIcon = document.getElementById("ctrlIcon");
    let currentTime = document.getElementById("currentTime");
    let duration = document.getElementById("duration");
    let songImg = document.getElementById("foto");

    song.onloadedmetadata = function () {
      progress.max = song.duration;
      progress.value = song.currentTime;
      duration.textContent = formatTime(song.duration);

    }

    function playPause() {
      if (ctrlIcon.classList.contains("fa-pause")) {
        song.pause();
        ctrlIcon.classList.remove("fa-pause")
        ctrlIcon.classList.add("fa-play")
        songImg.classList.remove("rotate")
      } else {
        song.play();
        ctrlIcon.classList.add("fa-pause")
        ctrlIcon.classList.remove("fa-play")
        songImg.classList.add("rotate")
      }
    }

    if (song.play()) {
      setInterval(() => {
        progress.value = song.currentTime
      }, 500);
    }

    progress.onchange = function () {
      song.play();
      song.currentTime = progress.value
      ctrlIcon.classList.add("fa-pause")
      ctrlIcon.classList.remove("fa-play")
    }

    song.addEventListener("timeupdate", function () {
      if (song.currentTime === song.duration) {
        song.currentTime = 0;
        song.pause();
        ctrlIcon.classList.remove("fa-pause")
        ctrlIcon.classList.add("fa-play")
        songImg.classList.remove("rotate")
      }
      progress.value = song.currentTime;
      currentTime.textContent = formatTime(song.currentTime);
    });

    progress.oninput = function () {
      song.currentTime = progress.value;
    }

    function formatTime(time) {
      const minutes = Math.floor(time / 60);
      const seconds = Math.floor(time % 60);
      return `${minutes}:${seconds.toString().padStart(2, '0')}`;
    }

    //audio visualizer
    window.onload = function () {

      var file = document.getElementById("thefile");
      var song = document.getElementById("song");
      var nomeArquivo = document.getElementById("nomeArquivo");
      var nomeBanda = document.getElementById("nomeBanda")

      file.onchange = function () {
        var files = this.files;
        song.src = URL.createObjectURL(files[0]);
        song.load();
        if (file.files.length > 0) {
          var nomeCompleto = file.files[0].name;
          var nomeSemExtensao = nomeCompleto.replace(".mp3", "");
          nomeArquivo.textContent = nomeSemExtensao;
          nomeBanda.textContent = ""
        } else {
          nomeArquivo.textContent = "Metal Bucetation"; // Valor padrão
        }

        var context = new AudioContext();
        var src = context.createMediaElementSource(song);
        var analyser = context.createAnalyser();

        var canvas = document.getElementById("canvas");
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
        var ctx = canvas.getContext("2d");

        src.connect(analyser);
        analyser.connect(context.destination);

        analyser.fftSize = 256;

        var bufferLength = analyser.frequencyBinCount;
        console.log(bufferLength);

        var dataArray = new Uint8Array(bufferLength);

        var WIDTH = canvas.width;
        var HEIGHT = canvas.height;

        var barWidth = (WIDTH / bufferLength) * 2.5;
        var barHeight;
        var x = 0;

        function renderFrame() {
          requestAnimationFrame(renderFrame);

          x = 0;

          analyser.getByteFrequencyData(dataArray);

          ctx.clearRect(0, 0, WIDTH, HEIGHT); // Limpa o canvas

          for (var i = 0; i < bufferLength; i++) {
            barHeight = dataArray[i];
          
            var r = 140 - (180 * (i / bufferLength));
            var g = 0;
            var b = 180 + (75 * (i / bufferLength));
          
            ctx.fillStyle = "rgba(" + r + "," + g + "," + b + ", 1)";
            ctx.fillRect(x, HEIGHT - barHeight, barWidth, barHeight);
          
            x += barWidth + 3;
          }
          
          ctx.fillStyle = "rgba(0, 0, 0, 0)"; // Definir a transparência para o retângulo preto
          ctx.fillRect(0, 0, WIDTH, HEIGHT);
        }
        renderFrame();
      };
    };
    //audio visualizer

    var arquivoInput = document.getElementById("arquivoInput");
    var icon = arquivoInput.querySelector("i");

    arquivoInput.addEventListener("mouseover", function() {
      icon.classList.add("fa-beat");
    });
    
    arquivoInput.addEventListener("mouseout", function() {
      icon.classList.remove("fa-beat");
    });