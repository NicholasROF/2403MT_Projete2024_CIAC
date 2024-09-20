// Conectar ao broker EMQX
const client = mqtt.connect('wss://broker.emqx.io:8084/mqtt'); // estabelece conexão com o broker no endereço

client.on('connect', function () { // se a conexão com o broker for bem sucedida 
console.log('Conectado ao broker EMQX'); // exibe console no navegador 
client.subscribe('Temperatura/Nick', function (err) { // fornece mensagem relacionada a temperatura
if (!err) { // se não houver erro
console.log('Inscrito no tópico Temperatura/Nick'); // mensagem de exibição
}
});
});

client.on('message', function (topic, message) { // se a mensagem 
const messagesDiv = document.getElementById('messages'); //
const newMessage = document.createElement('p'); //
newMessage.textContent = `Tópico: ${topic} - Mensagem: ${message.toString()}`; //
messagesDiv.appendChild(newMessage); //
});

function sendMessage() { //
   const messageInput = document.getElementById('messageInput'); //
   const message = messageInput.value; //
   client.publish('Temperatura/Nick', message); //
   messageInput.value = ''; //
}
function sendMessageTemp() { //
   const messageInput = document.getElementById('tempInput'); //
   const message = messageInput.value; //
   client.publish('Temperatura/Nick', message); //
}

function sendMessageTemp2() { //
   const messageInput = document.getElementById('temp2Input'); //
   const message = messageInput.value; //
   client.publish('Temperatura/Nick', message); //
}
               
    function ligar() {
       alert("Ar condicionado ligado!");
           // adicionar qualquer lógica adicional para "ligar" a luz.
       }

       function desligar() {
           alert("Arcondicionado desligado !");
           // adicionar qualquer lógica adicional para "desligar" a luz.
               }