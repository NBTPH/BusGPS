//This script will runs on the client, not the server
function convertToDMS(decimal, isLat) {
    const absolute = Math.abs(decimal);
    const degrees = Math.floor(absolute);
    const minutesNotTruncated = (absolute - degrees) * 60;
    const minutes = Math.floor(minutesNotTruncated);
    const seconds = ((minutesNotTruncated - minutes) * 60).toFixed(2);

    let direction = "";
    if(isLat){
        direction = decimal >= 0 ? "N" : "S";
    }
    else{
        direction = decimal >= 0 ? "E" : "W";
    }

    return `${degrees}° ${minutes}' ${seconds}" ${direction}`;
}

async function update_debug(){
    try{
        const response = await fetch('/get_debug');
        const msg_string = await response.json();

        document.getElementById('Roll').innerText = msg_string.Roll.toFixed(5);
        document.getElementById('Pitch').innerText = msg_string.Pitch.toFixed(5);
        document.getElementById('Yaw').innerText = msg_string.Heading_Raw.toFixed(5);
        document.getElementById('Vel_N').innerText = msg_string.Vel_N.toFixed(5);
        document.getElementById('Vel_E').innerText = msg_string.Vel_E.toFixed(5);
        document.getElementById('Pos_N').innerText = msg_string.Pos_N.toFixed(5);
        document.getElementById('Pos_E').innerText = msg_string.Pos_N.toFixed(5);
        document.getElementById('Lat_Raw').innerText = msg_string.Lat_Raw.toFixed(5);
        document.getElementById('Lon_Raw').innerText = msg_string.Lon_Raw.toFixed(5);
        
    }
    catch(err){
        console.error('Failed to parse data', err);
    }
}

async function update_data(){
    try{
        const response = await fetch('/get_data');
        const msg_string = await response.json();

        document.getElementById('Vehicle_ID').innerText = msg_string.id;

        const date = msg_string.date;
        document.getElementById('Date').innerText = `${date.day}/${date.month}/${date.year}`;

        const time = msg_string.utc;
        let hrs = time.hours + 7; 
        if(hrs > 24){
            hrs = hrs - 24;
        }
        const hrs_str = String(hrs).padStart(2, '0'); //pad to make it look nicer
        const mins = String(time.minutes).padStart(2, '0');
        const secs = time.seconds.toFixed(2); //keep 2 decimal places
        document.getElementById('UTC').innerText = `${hrs_str}:${mins}:${secs}`;

        document.getElementById('Lat').innerText = convertToDMS(msg_string.lat, true);
        document.getElementById('Lon').innerText = convertToDMS(msg_string.lon, false);
        document.getElementById('Heading').innerText = msg_string.heading;
        //we have to implement the html to know what id to use

        document.getElementById('Ignition').innerText = msg_string.ignition ? "ON" : "OFF";
        document.getElementById('Door').innerText = msg_string.doorOpen ? "OPEN" : "CLOSED";
        document.getElementById('AC').innerText = msg_string.ac ? "ON" : "OFF";
    }
    catch(err){
        console.error('Failed to parse data', err);
    }
}

async function update_status(){
    try{
        update_debug();
        const response = await fetch('/get_status');
        const status = await response.text(); //respond will be a character "1" or "0"
        
        const statusEl = document.getElementById('Status');
        if(status === "1"){ //if status is ok
            statusEl.innerText = "NO GPS FIX";
            statusEl.className = "status-bad"; //display that GPS is not good
        }
        else if(status === "2"){
            statusEl.innerText = "SENSOR IS CALIBRATING";
            statusEl.className = "status-calib"; //display that sensor is calibrating
        }
        else{
            statusEl.innerText = "GPS FIXED - Live Data";
            statusEl.className = "status-good";
            update_data(); //call to get and update data
        }
    }
    catch(err){
        console.error('Connection to ESP32 lost', err);
        const statusEl = document.getElementById('Status');
        statusEl.innerText = "ESP32 DISCONNECTED";
        statusEl.className = "status-bad";
    }
}

setInterval(update_status, 200); //get data every 500ms
update_status()