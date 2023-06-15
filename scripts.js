    let progress = document.getElementById("progress");
    let song = document.getElementById("song");
    let ctrlIcon = document.getElementById("ctrlIcon");
    let currentTime = document.getElementById("currentTime");
    let duration = document.getElementById("duration");
    let songImg = document.getElementById("foto");
    let volumeSlider = document.getElementById("volume");


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

    volumeSlider.oninput = function() {
      song.volume = volumeSlider.value / 100; // Defina o volume com base no valor do controle deslizante (0 a 1)
    };

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

        // Função para obter a imagem relacionada ao nome do arquivo
function getImageFromGoogle(filename) {
  var apiKey = 'AIzaSyBfJZRCvq4SqY-sN9yBi04W077XaehKb2w';
  var searchQuery = filename; // Adicione "capa do álbum" para melhorar a precisão da pesquisa
  var url = `https://www.googleapis.com/customsearch/v1?key=${apiKey}&cx=c6a46b6ea28a3411a&q=${encodeURIComponent(searchQuery)}&searchType=image`;

  axios.get(url)
    .then(function(response) {
      var items = response.data.items;
      if (items && items.length > 0) { // Verifica se items está definido e não vazio
        var imageUrl = items[0].link;
        console.log('URL da imagem:', imageUrl);
        console.log('Pesquisa:', searchQuery)
        var songImg = document.getElementById('foto'); // Obtém a referência da tag de imagem
        songImg.onload = function() {
          // Atribui a URL da imagem após o carregamento bem-sucedido
          songImg.setAttribute('src', imageUrl);
        };
        songImg.onerror = function() {
          console.log('Erro ao carregar a imagem:', imageUrl);
        };
        songImg.src = imageUrl; // Define o atributo src da tag de imagem
      } else {
        console.log('Nenhuma imagem encontrada para o nome do arquivo:', filename);
      }
    })
    .catch(function(error) {
      console.log('Erro ao obter a imagem:', error);
    });
}

    //audio visualizer
    window.onload = function () {

      var file = document.getElementById("thefile");
      var song = document.getElementById("song");
      var nomeArquivo = document.getElementById("nomeArquivo");
      var nomeBanda = document.getElementById("nomeBanda")

      file.onchange = function () {
        ctrlIcon.classList.remove("fa-pause")
        ctrlIcon.classList.add("fa-play")
        songImg.classList.remove("rotate")
        var files = this.files;
        song.src = URL.createObjectURL(files[0]);
        song.load();
        if (file.files.length > 0) {
          var nomeCompleto = file.files[0].name;
          var nomeSemExtensao = nomeCompleto.substring(0, nomeCompleto.lastIndexOf('.'));
          nomeArquivo.textContent = nomeSemExtensao;
          nomeBanda.textContent = ""
          getImageFromGoogle(nomeSemExtensao);
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

