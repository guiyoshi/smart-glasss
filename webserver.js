///////////////////////////////////////// Configuração Inicial /////////////////////////////////////////
var fs = require('file-system');
const http = require('http');
const util = require('util');
const server = http.createServer();
const imagePath = './resources/image.jpeg';
const audioPath = 'resources/labels.mp3';

//Importando APIs
const vision = require('@google-cloud/vision');
const textToSpeech = require('@google-cloud/text-to-speech');
const {Translate} = require('@google-cloud/translate').v2;

//Credenciais da conta de serviço Google
process.env['GOOGLE_APPLICATION_CREDENTIALS'] = "credentials/vision_api_credentials.json"

// Cria um client para o Text to Speech
const client_speech = new textToSpeech.TextToSpeechClient();
const client_vision = new vision.ImageAnnotatorClient();
const translate = new Translate();
const language = 'pt-br'

///////////////////////////////////////// Funções /////////////////////////////////////////

//Função Vision
async function visionAPI() {
  
	// Performs label detection on the image file
	const [result] = await client_vision.labelDetection(imagePath);
	const labels = result.labelAnnotations;
	var o = [];
	labels.slice(0,3).forEach(label => {
		o.push({description: label.description, score: label.score});
	});
	const inicio_fala = 'Objects detected: '
	const label_1 = labels[0]['description']
	const label_2 = labels[1]['description']
	const label_3 = labels[2]['description']
	const text = inicio_fala.concat(label_1,',',label_2,',',label_3)
	console.log('--- Detecção de objetos ---')
	console.log(`${text}`)
	console.log('')
	translateText(text)
	return o;
  }

//Função Tradutor
async function translateText(text) {

	let [translations] = await translate.translate(text, language);
	translations = Array.isArray(translations) ? translations : [translations];
	console.log('--- Tradução: ---');
	translations.forEach((translation, i) => {
	  console.log(`${translation}`);
	  console.log('');
	speechAPI(translation)
	});
  }

//Função Text to speech
async function speechAPI(text_label) {
  // The text to synthesize
  const text = text_label;

  // Construct the request
  const request = {
    input: {text: text},
    // Select the language and SSML voice gender (optional)
    voice: {languageCode: language, ssmlGender: 'NEUTRAL'},
    // select the type of audio encoding
    audioConfig: {audioEncoding: 'MP3'},
  };

  // Performs the text-to-speech request
  const [response] = await client_speech.synthesizeSpeech(request);
  // Write the binary audio content to a local file
  const writeFile = util.promisify(fs.writeFile);
  console.log('--- Audio: ---');
  console.log('Arquivo MP3 salvo em resources/label.mp3');
  console.log('-------------------------------------------');
  console.log('');
  await writeFile(audioPath, response.audioContent, 'binary');
  
};


///////////////////////////////////////// Servidor /////////////////////////////////////////
server.on('request', (request, response)=>{
	if(request.method == 'POST' && request.url == "/imageUpdate"){
		
		var ImageFile = fs.createWriteStream(imagePath, {encoding: 'utf8'});
		request.on('data', function(data){
			ImageFile.write(data);
		});

		request.on('end',async function(){
			ImageFile.end();
			const labels = await visionAPI();
			response.writeHead(200, {'Content-Type' : 'application/json'});
			response.end(JSON.stringify(labels));
		});
	}
	
	else if(request.url == '/audio'){
		var stat = fs.statSync(audioPath);
    	response.writeHead(200, {
        	'Content-Type': 'audio/mp3',
        	'Content-Length': stat.size
    });
    var readStream = fs.createReadStream(audioPath);
    readStream.pipe(response);
	}
	
	else{
		console.log("error");
		response.writeHead(405, {'Content-Type' : 'text/plain'});
		response.end();
	}
});

const port = 9000;
server.listen(port)
console.log(`--> Servidor atuando na porta ${port}`)
console.log('')