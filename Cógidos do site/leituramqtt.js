// Conectar ao broker EMQX
const client = mqtt.connect('wss://broker.emqx.io:8084/mqtt'); // estabelece conexão com o broker no endereço

client.on('connect', function () {
    console.log('Conectado ao broker EMQX');
    client.subscribe("CIAC/TEMP", function (err) {
        if (!err) {
            console.log('Inscrito no tópico Temperatura/Nick');
        }
    });
    client.subscribe("CIAC/PRES", function (err) {
        if (!err) {
            console.log('Inscrito no tópico Temperatura/Nick');
        }
    });
    client.subscribe("CIAC/ENER", function (err) {
        if (!err) {
            console.log('Inscrito no tópico Temperatura/Nick');
        }
    });
    client.subscribe("CIAC/LUMI", function (err) {
        if (!err) {
            console.log('Inscrito no tópico Temperatura/Nick');
        }
    });
  
});

// Variável para manter o estado do ar-condicionado entre leituras
var ligado = false;

if(ligado){
    document.getElementById('mensagemAIR_CONT_ON').textContent = "Ligado"; // Ar-condicionado ligado
}
else{
    document.getElementById('mensagemAIR_CONT_ON').textContent = "Desligado"; // Ar-condicionado desligado
}

client.on('message', function (topic, message) {
    if (topic == "CIAC/TEMP") {
        // Atualiza a temperatura na interface
        document.getElementById('mensagemTemp1').textContent = `${message.toString()} °C`;
        document.getElementById('mensagemTemp2').textContent = `${message.toString()} °C`;

        // Converte a mensagem recebida para número
        const temperatura = parseFloat(message.toString());

        // Obtém o valor do input de temperatura
        const tempInputValue = parseFloat(document.getElementById('tempInput').value);
        const temp2InputValue = parseFloat(document.getElementById('temp2Input').value);
        console.log(temp2InputValue);
        console.log(tempInputValue);
        console.log(temperatura);

        // Histerese: Muda o estado apenas quando os limites forem ultrapassados
        if (temperatura >= tempInputValue && !ligado) {
            console.log("Liga");
            ligado = true;
            document.getElementById('mensagemAIR_CONT_ON').textContent = "Ligado"; // Ar-condicionado ligado
            console.log("Ar-condicionado Ligado");
        } else if (temperatura < temp2InputValue && ligado) {
            console.log("desLiga");
            ligado = false;
            document.getElementById('mensagemAIR_CONT_ON').textContent = "Desligado"; // Ar-condicionado desligado
            console.log("Ar-condicionado Desligado");
        }
    }
    

      if (topic == "CIAC/PRES") {
        // Atualiza a presença na interface
        const presenca = parseFloat(message.toString());
        document.getElementById('mensagemPres').textContent = `${presenca}`;

        // Se a presença for maior ou igual a 1, aciona a luminosidade e mostra "Ligado"
        if (presenca >= 1) {
            ligar();  // Função para ligar a luz (atualiza a interface)
            document.getElementById('mensagemLumi').textContent = "Ligado";  // Atualiza o status da luminosidade
            console.log("Luz ligada automaticamente devido à presença");
        }
    }

    if (topic == "CIAC/ENER") {
        const agora = new Date();
        const dias = agora.getDay();
        const horas = agora.getHours();
        const minutos = agora.getMinutes();
        console.log(`Horário atual: ${dias}:${horas}:${minutos}`);
        document.getElementById('mensagemEner1').textContent = `${message.toString()} Wh`;
        document.getElementById('mensagemEner2').textContent = `${message.toString()} Wh`;
    }

    if (topic == "CIAC/LUMI") {
        document.getElementById('mensagemLumi').textContent = `${message.toString()}`;
    }

   
});
