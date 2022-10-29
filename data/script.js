// Complete project details: https://randomnerdtutorials.com/esp32-web-server-websocket-sliders/



var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getValues() {
    websocket.send("getValues");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
var currentHour;
var currentMinutes;
var currentSeconds;
function onOpen(event) {
    console.log('Connection opened');
    getValues();
    const now = new Date();
    currentHour = now.getHours();
    currentMinutes = now.getMinutes();
    currentSeconds = now.getSeconds();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateSliderPWM(element) {
    var sliderNumber = element.id.charAt(element.id.length - 1);
    var sliderValue = document.getElementById(element.id).value;
  //  document.getElementById("sliderValue" + sliderNumber).innerHTML = sliderValue;
    console.log(sliderValue);
    websocket.send(sliderNumber + "s" + sliderValue.toString());
    timeDone=false;
    rotDone=false;
    stepsDone=false;
    RestartTimer(-2);

}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        document.getElementById(key).innerHTML = myObj[key];
        document.getElementById("slider" + (i + 1).toString()).value = myObj[key];
    }
}



// 👇️ from CURRENT DATE

var timeDone = false;
//setInterval(function () {
//    const now = new Date();
//    if (document.getElementById("slider3").value == 0) {
//        document.getElementById("Minutes").innerHTML = now.getMinutes() - currentMinutes;
//        var seconds = now.getSeconds() - currentSeconds;
//        if (seconds < 0) {
//            seconds = 60 + seconds;
//        }
//        document.getElementById("Seconds").innerHTML = seconds;
//    }
//    else {
//        document.getElementById("Minutes").innerHTML = document.getElementById("slider3").value - (now.getMinutes() - currentMinutes) -1;
//
//        var seconds = now.getSeconds() - currentSeconds;
//        if (seconds < 0) {
//            seconds = 60 + seconds;
//        }
//        seconds = 60-seconds;
//        document.getElementById("Seconds").innerHTML = seconds;
//        if(seconds<2 && document.getElementById("slider3").value - (now.getMinutes() - currentMinutes) -1 == 0 && timeDone==false)
//        {
//            FadeIn();
//            timeDone=true;
//        }
//    }
//}, 1000);


var seconds =0;
var minutesTime =0;
setInterval(function () {
    const now = new Date();
    seconds += 1;
    //if(seconds==60){seconds=0; minutesTime+= 1;}

    if (document.getElementById("slider3").value == 0) {
        document.getElementById("TimerName").innerHTML = "Sekunden: ";
        if(seconds<10){document.getElementById("Seconds").innerHTML = seconds;}else{
        document.getElementById("Seconds").innerHTML = seconds;}
    }
    else{
        document.getElementById("TimerName").innerHTML = "Sekunden &uuml;brig: ";
        document.getElementById("Seconds").innerHTML = document.getElementById("slider3").value-seconds;
        if(document.getElementById("slider3").value-seconds < 1 && timeDone==false) //wenn Zeit vorbei
        {
            FadeIn();
            timeDone=true;
            document.getElementById("goal").innerHTML="Zeit erreicht!";
        }
    }
}, 1000);


function RestartTimer(Timer)
{
    seconds = Timer;
    timeDone=false;
}

function FadeOut() {
    document.getElementById('OverAll').style.animation = "fadeOut 1s linear";
    setTimeout(() => {
        document.getElementById('OverAll').style.visibility = "hidden";
        document.getElementById('OverAll').style.opacity = "0";
    }, 1000);
}


function FadeIn() {
    document.getElementById('OverAll').style.animation = "fadeIn 1s linear";
    setTimeout(() => {
        document.getElementById('OverAll').style.visibility = "visible";
        document.getElementById('OverAll').style.opacity = "1";
    }, 1000);
}
