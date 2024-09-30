
// **Conectar ao broker EMQX
const client1 = mqtt.connect('wss://broker.emqx.io:8084/mqtt'); // estabelece conexão com o broker no endereço

client1.on('connect', function () {
    console.log('Conectado ao broker EMQX');
    client1.subscribe('Temperatura/Nick', function (err) {
        if (!err) {
            console.log('Inscrito no tópico Temperatura/Nick');
        }
    });
});

client1.on('message', function (topic, message) {
    const messagesDiv = document.getElementById('messages');
    const newMessage = document.createElement('p');
    //const new1Message = document.createElement('p');
    newMessage.textContent = `Temperatura ${acionamento.toString()}: ${message.toString()}°C`;
    //new1Message.textContent = `Temperatura de acimonamento: ${message.toString()}°C`;

    messagesDiv.appendChild(newMessage);
    
    // Atualizar a célula de análise de dados para o parâmetro Ar Condicionado
    document.getElementById('mensagemAIR_TEMP').textContent = message.toString()+" °C";
});

function sendMessage() {
    const messageInput = document.getElementById('messageInput');
    const message = messageInput.value;

    client1.publish('CIAC/AIR_TEMP', message);
    acionamento = "do ar-condcionado"

    // Atualiza a célula de análise de dados para o parâmetro Ar Condicionado
    document.getElementById('mensagemAIR_TEMP').textContent = message + " °C";

    messageInput.value = ''; // Limpa o campo de entrada
}

function sendMessageTemp() {
    const messageInput = document.getElementById('tempInput');
    const message = messageInput.value;
    acionamento = "de acionamento máxima";
    console.log("leu");
    client1.publish('CIAC/AIR_CONT_ON', message);
}

function sendMessageTemp2() {
    const messageInput = document.getElementById('temp2Input');
    const message = messageInput.value;
    acionamento = "de acionamento mínima";
    console.log("leu");
    client1.publish('CIAC/AIR_CONT_OFF', message);
}
