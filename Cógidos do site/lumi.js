
    function ligar() {
        client1.publish('CIAC/ON_LUMI', true); // Publica para desligar a luz    
        document.getElementById("mensagemLumi").textContent = "Ligado"; // Atualiza status para Ativo
    }

    function desligar() {
        client1.publish('CIAC/ON_LUMI', false); // Publica para desligar a luz
        document.getElementById("mensagemLumi").textContent = "Desligado"; // Atualiza status para Desativo
    }

    var desligado = false;
// Função de Shutdown
function shutdownSystem() {
    if (!desligado){
    
    // Desligar a luz
    alert("Sistema em Shutdown. Luz e Ar-condicionado desligados.");
    document.getElementById("mensagemLumi").textContent = "Desligado"; // Atualiza status da luz para Desligado
    document.getElementById("mensagemAIR_CONT_ON").textContent = "Desligado"; // Atualiza status do ar-condicionado para Desligado
    
    // Enviar mensagens de desligamento via MQTT
    client1.publish('CIAC/SHUT', false); // Publica para desligar a luz e o ar-condicionado

    // Resetar os inputs de controle de temperatura
    document.getElementById('tempInput').value = '';
    document.getElementById('temp2Input').value = '';

    desligado = true;
    } else{
        
        // Desligar a luz
        alert("Sistema em Shutdown. Luz e Ar-condicionado ligados.");
        document.getElementById("mensagemLumi").textContent = "Ligado"; // Atualiza status da luz para Desligado
        document.getElementById("mensagemAIR_CONT_ON").textContent = "Ligado"; // Atualiza status do ar-condicionado para Desligado
        
        // Enviar mensagens de desligamento via MQTT
        client1.publish('CIAC/SHUT', false); // Publica para desligar a luz
     
    
        // Resetar os inputs de controle de temperatura
        document.getElementById('tempInput').value = '';
        document.getElementById('temp2Input').value = '';
    
        desligado = false;
    }
}

function sendMessage() {
    // Obter o valor inserido no input
    var message = document.getElementById("messageInput").value;
 
    // Exibir o valor digitado no campo de "Análise de dados" para "Ar Condicionado"
    document.getElementById("messageInput").innerText = message;
 
    // Atualizar a célula da tabela com o valor digitado
    document.getElementById("messageInput").innerText = message;
    document.getElementById("mensagemAIR_TEMP").innerText = message;
 }

