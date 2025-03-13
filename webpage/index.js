// ======================== global variables =============================
    // orientation constants (roll, pitch, yaw) in degrees, altitude in m;
    let roll = 0, pitch = 0, yaw= 0, altitude = 0;

    // some settings
    let rollMax = 45;
    let pitchMax = 45;
    let yawMax = 45;
    let speedMin = 1000;
    let speedMax = 2000;
    let basethrottle = 1000;
    
    let drone_mode = 0;
    const drone_modes = ["STABLE", "GUIDED", "AUTO", "CALIB", "MOTOR_TEST", "LAND"];
    let motorsInOrder = [1, 2, 3, 4];
    let motorsSelectionStatus = [0, 0, 0, 0];
    let motors_speed = 1000;

    let setpoints = [0, 0, 0, 0];

    let record_orientation = 0;
    let currentCalibStep = 0;
    let currentCalib = "imu"; // "imu" or "esc" 

    let roll_P = 10, pitch_P = 10, yaw_P = 10;
    let roll_D = 0, pitch_D = 0, yaw_D = 0;
    let roll_I = 0, pitch_I = 0, yaw_I = 0;

    // for blackbox related variables
    let timestamp = Date.now();  // Get current timestamp
    let error1 = 0;
    let error2 = 0;
    let error3 = 0;
    let error4 = 0;
    let pid_out1 = 0;
    let pid_out2 = 0;
    let pid_out3 = 0;
    let pid_out4 = 0;
    let motor_speed1 = 0;
    let motor_speed2 = 0;
    let motor_speed3 = 0;
    let motor_speed4 = 0;
    

// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // JOY CONTROLLER THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
let isManual = true;
let destroyJoystick1, destroyJoystick2;

document.getElementById("mode").addEventListener("click", function() {
    console.log("Current mode:", isManual ? "Manual" : "Auto");

    if (isManual) {
        setMode(1); // 1->GUIDED
        document.getElementById("current-mode").textContent = "Manual";
        this.textContent = "Switch to Auto";
        console.log("Mode changed to:", "Manual");
        document.getElementById("joy_controller").style.display = "flex";

        // Setup joysticks and store destroy functions
        destroyJoystick1 = setupJoystick("joystick1", "knob1");
        destroyJoystick2 = setupJoystick("joystick2", "knob2");

        isManual = false;  // Toggle mode
    } else {
        setMode(2); // 2->AUTO
        document.getElementById("current-mode").textContent = "Auto";
        this.textContent = "Switch to Manual";
        console.log("Mode changed to:", "Auto");

        // Destroy joystick event listeners
        if (destroyJoystick1) destroyJoystick1();
        if (destroyJoystick2) destroyJoystick2();

        isManual = true;  // Toggle mode
    }
});



document.getElementById("fullscreen").addEventListener("click", function() {
    if (!document.fullscreenElement) {
        document.documentElement.requestFullscreen();
        this.textContent = "Exit Fullscreen";
    } else {
        document.exitFullscreen();
        this.textContent = "Enter Fullscreen";
    }
});

function setupJoystick(containerId, knobId) {
    let value_index = 0;
    if ("joystick1"===containerId){
        value_index = 0;
    }else if("joystick2"===containerId){
        value_index = 2;
    }
    const container = document.getElementById(containerId);
    const knob = document.getElementById(knobId);
    const containerRadius = container.offsetWidth / 2;
    const knobRadius = knob.offsetWidth / 2;
    const maxDistance = containerRadius - knobRadius;
    let isDragging = false;

    container.addEventListener("mousedown", (e) => startDrag(e));
    container.addEventListener("touchstart", (e) => startDrag(e.touches[0]));

    document.addEventListener("mousemove", (e) => isDragging && moveKnob(e));
    document.addEventListener("touchmove", (e) => isDragging && moveKnob(e.touches[0]));

    document.addEventListener("mouseup", stopDrag);
    document.addEventListener("touchend", stopDrag);

    function startDrag(e) {
        isDragging = true;
        moveKnob(e);
    }

    function moveKnob(e) {
        const rect = container.getBoundingClientRect();
        const centerX = rect.left + containerRadius;
        const centerY = rect.top + containerRadius;

        let dx = e.clientX - centerX;
        let dy = e.clientY - centerY;
        const distance = Math.min(Math.sqrt(dx * dx + dy * dy), maxDistance);

        if (distance > 0) {
            const angle = Math.atan2(dy, dx);
            dx = Math.cos(angle) * distance;
            dy = Math.sin(angle) * distance;
        }

        knob.style.left = `${containerRadius + dx}px`;
        knob.style.top = `${containerRadius + dy}px`;

        const normalizedX = Math.round((dx / maxDistance) * 100);
        const normalizedY = Math.round((-dy / maxDistance) * 100);

        console.log(containerId, "X:", normalizedX, "Y:", normalizedY);

        document.getElementById(containerId+'x').innerText = normalizedX;
        document.getElementById(containerId+'y').innerText = normalizedY;

        // update the setpoints
        setpoints[value_index] = normalizedX;
        setpoints[value_index+1] = normalizedY;
        send_setpoints();
    }

    function stopDrag() {
        isDragging = false;
        knob.style.left = `${containerRadius}px`;
        knob.style.top = `${containerRadius}px`;
        console.log(containerId, "X: 0, Y: 0");
        document.getElementById(containerId+'x').innerText = 0;
        document.getElementById(containerId+'y').innerText = 0;

        // update the setpoints
        setpoints[value_index] = 0;
        setpoints[value_index+1] = 0;
        send_setpoints();
    }
    stopDrag();

    // Function to remove event listeners
    return function destroyJoystick() {
        container.removeEventListener("mousedown", startDrag);
        container.removeEventListener("touchstart", (e) => startDrag(e.touches[0]));

        document.removeEventListener("mousemove", moveKnob);
        document.removeEventListener("touchmove", (e) => moveKnob(e.touches[0]));

        document.removeEventListener("mouseup", stopDrag);
        document.removeEventListener("touchend", stopDrag);
        
        console.log(`Joystick ${containerId} destroyed`);

        document.getElementById("joy_controller").style.display = "none";
    };
}



// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // NAVBAR RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
function show_or_hide_block(div_id) {
    if(drone_mode === 2){ // 2->AUTO
        alert('drone is in auto mode, switch to manual mode to switch tab');
    }else if(drone_mode === 4){ // 4->MOTOR_TEST
        alert('drone is in motor testing mode, switch to manual mode')
    }else if(altitude > 0.5){ 
        alert('altitude is higher that 0.5m');
    }
    else if(connection_status == 0){
        alert('not connected');
    }
    else{
        // Get all the divs in an array
        const divs = ['drone_info', 'calibration', 'pid_inputs', 'joy', 'settings', "log"];
        
        // Loop through each div and hide it
        divs.forEach(id => {
            const element = document.getElementById(id);
            element.style.display = 'none';

            const element_tab = document.getElementById(id+"_tab");
            element_tab.style.fontWeight = 'normal';
            element_tab.style.color = 'black';
        });

        // Display the clicked div
        const selectedDiv = document.getElementById(div_id);
        selectedDiv.style.display = 'flex';

        const selectedDivTab = document.getElementById(div_id+"_tab");
        selectedDivTab.style.fontWeight = 'bold';
        selectedDivTab.style.color = 'red';


        // change the drone mode based on the tab
        // mode(0->STABLE, 1->GUIDED, 2->AUTO, 3->CALIB, 4-> MOTOR_TEST, 5->LAND)
        switch (div_id){
            case 'drone_info':
                setMode(0); // 4-> motor_testing
                break;
            case 'calibration':
                setMode(3); // 3->calibration
                break;
            case 'pid_inputs':
                // // Display the log div to store the logs while drone is tuning
                // const selectedDiv = document.getElementById('log');
                // selectedDiv.style.display = 'flex';

                getPIDValues();
                setMode(1); // 1->guided
                break;
            case 'joy':
                setMode(1); // 1->guided
                break;
            case 'settings':
                // no need to set any mode
                // setMode();
                break;
            case 'log': 
                // no need to set any mode
                // setMode();
                break;
        }
        // clear the joystick block
        isManual = 1;
        document.getElementById("mode").textContent = "Switch to Manual";
        document.getElementById("current-mode").textContent = "Stable";
        // Destroy joystick event listeners
        if (destroyJoystick1) destroyJoystick1();
        if (destroyJoystick2) destroyJoystick2();
    }
}

let ip_address = "esp32.local";
ip_address = "10.42.0.42";
const ipDiv = document.getElementById("ip_address_span");
const conn_status_div = document.getElementById("connection_status");
ipDiv.innerText = ip_address;

function changeIp() {
    ip_address = ipDiv.innerText;
    alert("IP Address changed to" + ip_address);
    if (socket && socket.readyState !== WebSocket.CLOSED) {
        socket.close();
    }
    connectWebSocket();
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// //
// // // WEBSOCKET RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
let socket, pongs_count = 0;
let connection_status=0;
let last_msg_time = Date.now();

function connectWebSocket() {
    socket = new WebSocket("ws://" + ip_address + ":81/");
    console.log("Connecting to WebSocket at ws://" + ip_address + ":81/");

    socket.onopen = function () {
        console.log("Connected to WebSocket server");
        conn_status_div.innerText = "Connected";
        connection_status = 1;
        conn_status_div.style.color = "Green";
        document.getElementById("droneMode").style.display = "block";

        // connection status checking
        setInterval(() => {
            if (Date.now() - last_msg_time >= 3000){
                handleDisconnect();
            }
        }, 3000);
    };

    socket.onmessage = function (event) {
        last_msg_time = Date.now();
        pongs_count += 1;
        // console.log("Server response: " + event.data);
        
        handleMsg(event.data);
    };

    socket.onerror = function (event) {
        console.error("WebSocket error observed:", event);
        handleDisconnect();
    };

    socket.onclose = function (event) {
        console.error("WebSocket is closed:", event);
        handleDisconnect();
    };
}

function handleDisconnect() {
    if (pongs_count >= 1) { 
        // alert("WebSocket disconnected");
        conn_status_div.innerText = "Disconnected";
        connection_status = 0;
        conn_status_div.style.color = "red";
        document.getElementById("droneMode").style.display = "none";
    } else {
        conn_status_div.innerText = "Not Connected";
        connection_status = 0;
        conn_status_div.style.color = "blue";
        document.getElementById("droneMode").style.display = "none";
    }


    // Attempt to reconnect after 3 seconds
    // setTimeout(connectWebSocket, 3000);
}

 //**Connect WebSocket when the page loads**
 window.onload = function () {
     connectWebSocket();
     updatePIDs_text();
 };


function send_data(msg){
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(msg);
        console.log("sent mesage: ", msg);
    } else {
        // alert("Disconnected from the drone");
        console.error("WebSocket connection is not open.");
    }
}

// Function to parse incoming messages
function handleMsg(msg){
    try {
      const data = JSON.parse(msg);

      if (data.ip){
          ip_address = data.ip;
          ipDiv.innerText = ip_address;
          console.log("ip address updated to " + ip_address);
          return;
      }

      const messageId = data.id;
      // Handle message based on message_id
      switch (messageId) {
          case -1: // -1(drone<-GCS): get_mode
            alert("-1(drone<-GCS): get_mode \n this step is only on drone side (drone<-GCS)");
            break;       
          case 0: // 0(drone<>GCS): set_modes(0->STABLE, 1->GUIDED, 2->AUTO, 3->CALIB, 4-> MOTOR_TEST, 5->LAND)
            drone_mode = data.mode;
            
            document.getElementById("droneMode").innerText = 'Mode : ' + drone_modes[drone_mode];
            break;
          case 1: // 1(drone<>GCS): get_PIDs
            console.log("Gor request for PID values");
            console.log("sending PID values");
            sendPIDValues(0);
            break;
          case 2: // 2(drone<>GCS): set_PIDs{save, proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw, base_throttle}
              console.log("updating the PID values");
              document.getElementById("rpmax").value = data.rollP*10;
              document.getElementById("rimax").value = data.rollI*10;
              document.getElementById("rdmax").value = data.rollD*10;
              document.getElementById("ppmax").value = data.pitchP*10;
              document.getElementById("pimax").value = data.pitchI*10;
              document.getElementById("pdmax").value = data.pitchD*10;
              document.getElementById("ypmax").value = data.yawP*10;
              document.getElementById("yimax").value = data.yawI*10;
              document.getElementById("ydmax").value = data.yawD*10;

              document.getElementById("rpmin").value = 0;
              document.getElementById("rimin").value = 0;
              document.getElementById("rdmin").value = 0;
              document.getElementById("ppmin").value = 0;
              document.getElementById("pimin").value = 0;
              document.getElementById("pdmin").value = 0;
              document.getElementById("ypmin").value = 0;
              document.getElementById("yimin").value = 0;
              document.getElementById("ydmin").value = 0;
              
              document.getElementById('rpValue').value = data.rollP;
              document.getElementById('riValue').value = data.rollI;
              document.getElementById('rdValue').value = data.rollD;
              document.getElementById('ppValue').value = data.pitchP;
              document.getElementById('piValue').value = data.pitchI;
              document.getElementById('pdValue').value = data.pitchD;
              document.getElementById('ypValue').value = data.yawP;
              document.getElementById('yiValue').value = data.yawI;
              document.getElementById('ydValue').value = data.yawD;
              document.getElementById('basethrottle').value = data.base_throttle;
            

              roll_P = data.rollP;
              roll_I = data.rollI;
              roll_D = data.rollD;
              pitch_P = data.pitchP;
              pitch_I = data.pitchI;
              pitch_D = data.pitchD;
              yaw_P = data.yawP;
              yaw_I = data.yawI;
              yaw_D = data.yawD;

              changeMinMax();

              updatePIDs_text();

              break;
          case 3: // 3(drone<>GCS): get_motors_order
            console.log("3(drone<-GCS): get_motors_order \n sending the motors order");
            set_motors_order(0);
            break;
          case 4: // 4(drone<-GCS): set_motors_order{save, top_left, top_right, bottom_left, bottom_right}
            let save_or_not = data.save;
            motorsInOrder[0] = data.top_left;
            motorsInOrder[1] = data.top_right;
            motorsInOrder[2] = data.bottom_left;
            motorsInOrder[3] = data.bottom_right;
            console.log("motors order " + motorsInOrder + "update them on the drone bomma");
            alert("motors order " + motorsInOrder + " update them on the drone bomma");
            break;
          case 5: // 5(drone<>GCS): motors_test{m1_state, m2_state, m3_state, m4_state, speed}
              // drone should be in MOTOR TESTING mode to apply those speed 
              // we need a method to find wether the motors are spinning or not
              console.log("motors are spinning");
              motor_test();
              // after leaving the motor test send STABLE MODE
              break;
          case 6: // 6(drone<>GCS): get_settings
              console.log("request for setting, so sending them");
              set_settings(0);
              break;
          case 7: // 7(drone<>GCS): set_settings{max_roll, max_pitch, max_yaw, min_speed, max_speed}
              rollMax = data.max_roll;
              pitchMax = data.max_pitch;
              yawMax = data.max_yaw;
              speedMin = data.min_speed;
              speedMax = data.max_speed;
              document.getElementById("roll-max").value = data.max_roll;
              document.getElementById("pitch-max").value = data.max_pitch;
              document.getElementById("yaw-max").value = data.max_yaw;
              document.getElementById("speed-max").value = data.min_speed;
              document.getElementById("speed-min").value = data.max_speed;
              console.log("settings updated "+ rollMax + pitchMax + yawMax + speedMin + speedMax);
              alert("settings updated "+ rollMax + " " + pitchMax + " " + yawMax + " " + speedMin + " " + speedMax);
              break;
          case 8: // 8(drone<-GCS): imu_calib_command{step}
            alert("8(drone<-GCS): imu_calib_command{step} \n this step is only on drone side (drone<-GCS)");
              break;
          case 9: // 9(drone->GCS): imu_calib_response{step, response}
            const current_step = data.step;
            const response =  data.response;

            handle_calib_response(current_step, response)
            break;
          case 10: // 10(drone<-GCS): get_orientation
              alert("10(drone<-GCS): get_orientation \n this step is only on drone side (drone<-GCS)");
              break;
          case 11: // 11(drone->GCS): drone_orientation{roll, pitch, yaw}
              // console.log("Got drone orienation from the drone, updating the values on this page")
              roll = data.roll;
              pitch = data.pitch;
              yaw = -data.yaw;
              altitude = data.altitude;

              updateDroneOrientation();
              break;
          case 12: // 12(drone<-GCS): setpoints{roll, pitch, yaw, altitude}
            alert("Message ID 12(drone<-GCS): setpoints{roll, pitch, yaw, altitude} \n this step is only on drone side (drone<-GCS)");
            break;
          case 13: // 13(drone<-GCS): motor_calib_command{step}
            alert("13(drone<-GCS): motor_calib_command{step} \n this  step is only on drone side (drone<-GCS)");
            break;
          case 14:{ // 14(drone->GCS): motor_calib_response{step, response}
            const current_step = data.step;
            const response =  data.response;

            handle_calib_response(current_step, response)
            break;}
          case 15:{ // 15(drone<-GCS): get_imu_offsets
            alert("Message ID 15(drone<-GCS): get_imu_offsets \n this step is only on drone side (drone<-GCS)");
          }
          
          case 16:{ // 16(drone->GCS): set_imu_offsets{roll_offset,pitch_offset,yaw_offset,accx_offset,accy_offset,accz_offset, roll_scalling, pitch_scalling, yaw_scalling, accx_scalling, accy_scalling, accz_scalling}
                document.getElementById("roll_offset").innerText = data.roll_offset;
                document.getElementById("pitch_offset").innerText = data.pitch_offset;
                document.getElementById("yaw_offset").innerText = data.yaw_offset;
                document.getElementById("accx_offset").innerText = data.accx_offset;
                document.getElementById("accy_offset").innerText = data.accy_offset;
                document.getElementById("accz_offset").innerText = data.accz_offset;
                document.getElementById("roll_scalling").innerText = data.roll_scalling;
                document.getElementById("pitch_scalling").innerText = data.pitch_scalling;
                document.getElementById("yaw_scalling").innerText = data.yaw_scalling;
                document.getElementById("accx_scalling").innerText = data.accx_scalling;
                document.getElementById("accy_scalling").innerText = data.accy_scalling;
                document.getElementById("accz_scalling").innerText = data.accz_scalling;
          }
          case 17:{// 17(drone->GCS): pid_status{timestamp, roll, pitch, yaw, altitude, error1, error2, error3, error4, pid_out1, pid_out2, pid_out3, pid_out4, motor_speed1, motor_speed2, motor_speed3, motor_speed4}
            console.log("Server response: " + data);
            
            timestamp = data.timestamp;
            roll = data.roll;
            pitch = data.pitch;
            yaw = -data.yaw;
            altitude = data.altitude;
            updateDroneOrientation();
            
            error1 = data.error1;
            error2 = data.error2;
            error3 = data.error3;
            error4 = data.error4;
            pid_out1 = data.pid_out1;
            pid_out2 = data.pid_out2;
            pid_out3 = data.pid_out3;
            pid_out4 = data.pid_out4;
            motor_speed1 = data.motor_speed1;
            motor_speed2 = data.motor_speed2;
            motor_speed3 = data.motor_speed3;
            motor_speed4 = data.motor_speed4;

            document.getElementById("error1").innerText = data.error1;
            document.getElementById("error2").innerText = data.error2;
            document.getElementById("error3").innerText = data.error3;
            document.getElementById("error4").innerText = data.error4;
            document.getElementById("pid_out1").innerText = data.pid_out1;
            document.getElementById("pid_out2").innerText = data.pid_out2;
            document.getElementById("pid_out3").innerText = data.pid_out3;
            document.getElementById("pid_out4").innerText = data.pid_out4;
            document.getElementById("motor_speed1").innerText = data.motor_speed1;
            document.getElementById("motor_speed2").innerText = data.motor_speed2;
            document.getElementById("motor_speed3").innerText = data.motor_speed3;
            document.getElementById("motor_speed4").innerText = data.motor_speed4;

            document.getElementById("pid_out1").style.color = (speedMax-speedMin < Math.abs(data.pid_out1)) ? "red":"black";
            document.getElementById("pid_out2").style.color = (speedMax-speedMin < Math.abs(data.pid_out2)) ? "red":"black";
            document.getElementById("pid_out3").style.color = (speedMax-speedMin < Math.abs(data.pid_out3)) ? "red":"black";
            document.getElementById("pid_out4").style.color = (speedMax-speedMin < Math.abs(data.pid_out4)) ? "red":"black";
            document.getElementById("motor_speed1").style.color = (0 < data.motor_speed1 || speedMax < data.motor_speed1) ? "black":"red";
            document.getElementById("motor_speed2").style.color = (0 < data.motor_speed2 || speedMax < data.motor_speed2) ? "black":"red";
            document.getElementById("motor_speed3").style.color = (0 < data.motor_speed3 || speedMax < data.motor_speed3) ? "black":"red";
            document.getElementById("motor_speed4").style.color = (0 < data.motor_speed4 || speedMax < data.motor_speed4) ? "black":"red";
            

            // sendToPython();

            if (record_orientation === 1){
                add_data_to_graph();
              }
            break;
          }
          default:
              console.log(`Unknown Message ID: ${messageId}`);
              break;
      }
  } catch (err) {
      console.log(`Error parsing message: ${err}`);
  }
  }

function sendPIDValues(save_or_not) {
    console.log("sent pid values");

    roll_P =  parseFloat(document.getElementById('rpValue').value) || 0;
    roll_I =  parseFloat(document.getElementById('riValue').value) || 0;
    roll_D =  parseFloat(document.getElementById('rdValue').value) || 0;
    pitch_P =  parseFloat(document.getElementById('ppValue').value) || 0;
    pitch_I =  parseFloat(document.getElementById('piValue').value) || 0;
    pitch_D =  parseFloat(document.getElementById('pdValue').value) || 0;
    yaw_P =  parseFloat(document.getElementById('ypValue').value) || 0;
    yaw_I =  parseFloat(document.getElementById('yiValue').value) || 0;
    yaw_D =  parseFloat(document.getElementById('ydValue').value) || 0;
    basethrottle = parseFloat(document.getElementById('basethrottle').value) || 0;
    const pidValues = {
        id: 2,
        save: save_or_not,
        rollP: roll_P || 0,
        rollI: roll_I || 0,
        rollD: roll_D || 0,
        pitchP: pitch_P || 0,
        pitchI: pitch_I || 0,
        pitchD: pitch_D || 0,
        yawP: yaw_P || 0,
        yawI: yaw_I || 0,
        yawD: yaw_D || 0,
        base_throttle: basethrottle || 0
    };

    updatePIDs_text();

    send_data(JSON.stringify(pidValues));
}

////////////////////////////
let syncPitchRoll = false;

document.addEventListener("DOMContentLoaded", function () {
    const toggleButton = document.createElement("button");
    toggleButton.innerText = "Sync Pitch & Roll";
    toggleButton.onclick = function () {
        syncPitchRoll = !syncPitchRoll;
        toggleButton.style.backgroundColor = syncPitchRoll ? "lightgreen" : "";
    };
    document.getElementById("pidRelatedBtn").appendChild(toggleButton);

    const pitchRollPairs = [
        ["rpValue", "ppValue"],
        ["riValue", "piValue"],
        ["rdValue", "pdValue"]
    ];


    document.getElementById("basethrottle").addEventListener("input", function () {
        sendPIDValues(0);
    });

    pitchRollPairs.forEach(([rollId, pitchId]) => {
        document.getElementById(rollId).addEventListener("input", function () {
            if (syncPitchRoll) {
                document.getElementById(pitchId).value = this.value;
            }
            sendPIDValues(0);
        });

        document.getElementById(pitchId).addEventListener("input", function () {
            if (syncPitchRoll) {
                document.getElementById(rollId).value = this.value;
            }
            sendPIDValues(0);
        });
    });
});

////////////////////////////
  
function getPIDValues(){
    const getPID = {
      id : 1,
    };
    send_data(JSON.stringify(getPID));
  }

function getMode(){
    const getMod = {
        id : -1,
    };
    send_data(JSON.stringify(getMod));
}

function setMode(mode_number){
    drone_mode = mode_number;
    const setMod = {
        id : 0,
        mode: mode_number,
    };
    send_data(JSON.stringify(setMod));
    document.getElementById("droneMode").innerText = 'Mode : ' + drone_modes[drone_mode];
}

function get_motors_order(){
    const getMot = {
      id : 3,
    };
    send_data(JSON.stringify(getMot));
}

function set_motors_order(save_or_not){
    console.log("sent command to update the motors order")
    const setMot = {
      id : 4,
      save: save_or_not,
      top_left : motorsInOrder[0],
      top_right : motorsInOrder[1],
      bottom_left : motorsInOrder[2],
      bottom_right : motorsInOrder[3],
    };
    send_data(JSON.stringify(setMot));
  }

function motor_test(){
    
    const testMotors = {
        id : 5,
        m1_state : motorsSelectionStatus[0],
        m2_state : motorsSelectionStatus[1],
        m3_state : motorsSelectionStatus[2],
        m4_state : motorsSelectionStatus[3],
        speed: motors_speed,
    };
    send_data(JSON.stringify(testMotors));
}

function get_settings(){
    const getMot = {
      id : 6,
    };
    send_data(JSON.stringify(getMot));
  }

function set_settings(save_or_not){
    rollMax = document.getElementById("roll-max").value;
    pitchMax = document.getElementById("pitch-max").value;
    yawMax = document.getElementById("yaw-max").value;
    speedMin = document.getElementById("speed-min").value;
    speedMax = document.getElementById("speed-max").value;
    const setMot = {
        id : 7,
        save: save_or_not,
        max_roll : rollMax,
        max_pitch : pitchMax,
        max_yaw : yawMax,
        min_speed : speedMin,
        max_speed : speedMax,
    };
    send_data(JSON.stringify(setMot));
    console.log("commanding to update settings");
}

function send_setpoints(){
    // roll, pitch, yaw, altitude
    const setpnts = {
      id : 12,
      roll : setpoints[0],
      pitch : setpoints[1],
      yaw : setpoints[2],
      altitude : setpoints[3],
    };
    send_data(JSON.stringify(setpnts));
    console.log("commanding to update setpoints " + setpoints);
}

function imu_calib_cmd(current_step){
    const calib_step = {
      id : 8,
      step : current_step,
    };
    send_data(JSON.stringify(calib_step));
    console.log("commanding to start imu calibration step " + current_step);
}

function motor_calib_cmd(current_step){
    const calib_step = {
      id : 13,
      step : current_step,
    };
    send_data(JSON.stringify(calib_step));
    console.log("commanding to start motor calibration step " + current_step);
  }

function handle_calib_response(current_step, response){
    let response_txt = "";
    if (response === 1){
        response_txt = "Step " + (current_step) + " is successful";
        document.getElementById('nextBtn').disabled = false; 
    }else if (response === 0){
        response_txt = "Step " + (current_step) + " is failed zero divison error, maybe imu not connected";
        document.getElementById('nextBtn').disabled = true;  
    }else if (response === -1){
        response_txt = "Step " + (current_step) + "may imu not connected";
        document.getElementById('nextBtn').disabled = true;    
    }
    document.getElementById('startBtn').disabled = false;
    document.getElementById('prevBtn').disabled = false;
    document.getElementById('resetBtn').disabled = false;

    document.getElementById("calib_step_response").innerText = response_txt;

    console.log(response_txt);
}

function get_imu_offsets(){
    const getMod = {
        id : 15,
    };
    send_data(JSON.stringify(getMod));
}

// // // connect to python script to analys the response
// let pythonSocket = null;

// function start_python_socket() {
//     pythonSocket = new WebSocket("ws://localhost:8765");

//     pythonSocket.onopen = function () {
//         console.log("Connected to PID suggester");

//         // Update the div content to remove the button and show connection status
//         document.getElementById("pidSuggestion").innerHTML = "Connected! Waiting for analysis...";
//     };

//     pythonSocket.onmessage = function (event) {
//         const response = JSON.parse(event.data);
//         console.log("PID Suggestion:", response.suggestion);

//         // Update suggestion in the div
//         document.getElementById("pidSuggestion").innerText = "PID Suggestion: " + response.suggestion;
//     };

//     pythonSocket.onclose = function () {
//         console.log("Disconnected from PID suggester");
//         document.getElementById("pidSuggestion").innerHTML =
//             '<button onclick="start_python_socket();">Reconnect to PID suggester</button>';
//     };

//     pythonSocket.onerror = function (error) {
//         console.error("WebSocket Error:", error);
//         document.getElementById("pidSuggestion").innerText = "Error connecting to PID suggester.";
//     };
// }

// // Function to send data to Python
// function sendToPython() {
//     if (pythonSocket && pythonSocket.readyState === WebSocket.OPEN) {
//         let data = {
//             timestamp: Date.now(),
//             roll_error: error1,  // Use actual roll error variable
//             pitch_error: error2,
//             yaw_error: error3
//         };

//         pythonSocket.send(JSON.stringify(data));
//     } else {
//         console.warn("Python WebSocket is not connected.");
//     }
// }



// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // CALIBRATION RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 

const esc_calib_steps = [
    { description: "Connect the battery to the flight controller(not motors).<br> WARNING: REMOVE PROPS BEFORE CONNECTING THE BATTERY.", visual: 'calibration_visuals/max_throttle.jpg', visual_type: 'image' },
    { description: "Sending MAX throttle...<br>Wait for ESCs to beep (indicating calibration mode).<br>Once the battery is connected, press 'Next' to continue...", visual: 'calibration_visuals/connect_battery.jpg', visual_type: 'image' },
    { description: "Sending MIN throttle...<br>ESCs should confirm calibration with another beep.", visual: 'calibration_visuals/min_throttle.jpg', visual_type: 'image' },
    { description: "Testing motor response. Slowly increasing throttle...<br>Motor test complete. If motors respond correctly", visual: 'calibration_visuals/calib_complete.jpg', visual_type: 'image' },
    { description: "calibration is successful.", visual: 'calibration_visuals/calib_complete.jpg', visual_type: 'image' }
];

const imu_calib_steps = [
    { description: 'Place the drone on the flat surface.', visual: 'calibration_visuals/flat_on_a_surface.png', visual_type: 'image' },
    { description: 'Place the drone on its back (i.e., pitch 90 degrees).', visual: 'calibration_visuals/on_backside.png', visual_type: 'image' },
    { description: 'Place the drone on the left/right side (i.e., roll 90 degrees).', visual: 'calibration_visuals/on_left_or_right_side.png', visual_type: 'image' },
    { description: 'Place the drone on the flat surface again.', visual: 'calibration_visuals/flat_on_a_surface.png', visual_type: 'image' },
    { description: `<b>Roll (X-axis)</b><br>The drone tilts to the left or right. <br>Think of the drone tipping over its sides.`, visual: 'calibration_visuals/roll.webm', visual_type: 'video' },
    { description: `<b>Pitch (Y-axis)</b><br>The nose of the drone tilts up or down. <br> Imagine the drone pivoting forward or backward, like nodding your head.`, visual: 'calibration_visuals/pitch.webm', visual_type: 'video' },
    { description: `<b>Yaw (Z-axis)</b><br>The drone rotates left or right while staying level. <br>It's like spinning around a vertical axis.`, visual: 'calibration_visuals/yaw.webm', visual_type: 'video' },
    { description: "IMU calibration is completed!", visual: 'calibration_visuals/calib_complete.jpg', visual_type: 'image' }
];

let currentCalibration = null;

// JavaScript logic to handle active step highlighting
const imuCalibBtn = document.getElementById('imu_calib');
const escCalibBtn = document.getElementById('esc_calib');

// Update the progress bar
const progressBar = document.getElementById('progressBar');


imuCalibBtn.addEventListener('click', () => {
    currentCalibration = imu_calib_steps;
    showCalibrationStep("IMU");

    imuCalibBtn.classList.add('active');
    escCalibBtn.classList.remove('active');
    currentCalib = "imu";
    document.getElementById("calib_step_response").innerText = "";
});

escCalibBtn.addEventListener('click', () => {
    currentCalibration = esc_calib_steps;
    showCalibrationStep("ESC");

    escCalibBtn.classList.add('active');
    imuCalibBtn.classList.remove('active');
    currentCalib = "esc";
    document.getElementById("calib_step_response").innerText = "";
});


function showCalibrationStep(calib_items) {
    document.getElementById('calib').style.display = 'block';
    document.getElementById('calibrateBtn').style.display = 'block';
    document.getElementById('prevBtn').style.display = 'none';
    document.getElementById('nextBtn').style.display = 'none';
    document.getElementById('resetBtn').style.display = 'none';
    document.getElementById('startBtn').style.display = 'none';
    document.getElementById('step-description').innerHTML = "Let's start the " + calib_items + " calibration :)";
    document.getElementById('step-visual').innerHTML = `<img src="../calibration_visuals/calib_intro.jpg" alt="Step Visual"/>`; 
    document.getElementById('step-number').innerHTML = '';
    
    // Reset the current step
    currentCalibStep = 1;
}

// Start button logic
document.getElementById('calibrateBtn').addEventListener('click', function() {
    showStep(currentCalibStep);
    document.getElementById('calibrateBtn').style.display = 'none';
    document.getElementById('prevBtn').style.display = 'inline-block';
    document.getElementById('startBtn').style.display = 'inline-block';
    document.getElementById('nextBtn').style.display = 'inline-block';
    document.getElementById('resetBtn').style.display = 'inline-block';

    // Disable 'Next' and 'Previous' buttons
    document.getElementById('prevBtn').disabled = true;
    document.getElementById('nextBtn').disabled = true;
});

// Previous button logic
document.getElementById('prevBtn').addEventListener('click', function() {
    if (currentCalibStep > 1) {
        currentCalibStep--;
        showStep(currentCalibStep);
    }
    // clear the response box
    document.getElementById("calib_step_response").innerText = "";

    // Enable or disabling the buttons
    document.getElementById('nextBtn').disabled = true;    
    document.getElementById('startBtn').disabled = false;
    document.getElementById('prevBtn').disabled = false;
    document.getElementById('resetBtn').disabled = false;

    if (currentCalibStep <= 1) {
        document.getElementById('prevBtn').disabled = true;
    }
});


// Start button logic
document.getElementById('startBtn').addEventListener('click', function() {
    // Disable all buttons
    document.getElementById('nextBtn').disabled = true;    
    document.getElementById('startBtn').disabled = true;
    document.getElementById('prevBtn').disabled = true;
    document.getElementById('resetBtn').disabled = true;

    document.getElementById("calib_step_response").innerText = "starting step " + currentCalibStep;

    if(currentCalib === "imu"){
        imu_calib_cmd(currentCalibStep);
    }else if(currentCalib === "esc"){
        motor_calib_cmd(currentCalibStep);
    }
});


// Next button logic
document.getElementById('nextBtn').addEventListener('click', function() {
    if (currentCalibStep < currentCalibration.length) {
        currentCalibStep++;
        showStep(currentCalibStep);
        document.getElementById('nextBtn').disabled = true;
    }

    // clear the response box
    document.getElementById("calib_step_response").innerText = "";

    // Enable or disabling the buttons
    document.getElementById('nextBtn').disabled = true;    
    document.getElementById('startBtn').disabled = false;
    document.getElementById('prevBtn').disabled = false;
    document.getElementById('resetBtn').disabled = false;
    
    if (currentCalibStep >= currentCalibration.length) {
        document.getElementById('startBtn').disabled = true;
        document.getElementById('nextBtn').disabled = true;
    }
});



// Reset button logic
document.getElementById('resetBtn').addEventListener('click', function() {
    document.getElementById('prevBtn').disabled = true;
    document.getElementById('resetBtn').disabled = true;
    document.getElementById('nextBtn').disabled = true;
    document.getElementById('startBtn').disabled = false;

    currentCalibStep = 1;
    showStep(currentCalibStep);
});

function showStep(stepIndex) {
    const step = currentCalibration[stepIndex-1];
    document.getElementById('step-description').innerHTML = step.description;
    document.getElementById('step-number').innerHTML = `Step ${stepIndex}`;
    if (step.visual_type === 'image') {
        document.getElementById('step-visual').innerHTML = `<img src="../${step.visual}" alt="Step Visual" width="300" />`;
    } else if (step.visual_type === 'video') {
        document.getElementById('step-visual').innerHTML = `<video autoplay loop><source src="../${step.visual}" type="video/webm"></video>`;
    }
    // Update the progress bar
    progressBar.style.width = (currentCalibStep) / currentCalibration.length * 100 + '%';
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // PID RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
function changeMinMax() {
    // roll value
    const r_pvalueDiv = document.getElementById("rpValue");
    const r_ivalueDiv = document.getElementById("riValue");
    const r_dvalueDiv = document.getElementById("rdValue");

    r_pvalueDiv.max = document.getElementById("rpmax").value;
    r_pvalueDiv.min = document.getElementById("rpmin").value;
    r_ivalueDiv.max = document.getElementById("rimax").value;
    r_ivalueDiv.min = document.getElementById("rimin").value;
    r_dvalueDiv.max = document.getElementById("rdmax").value;
    r_dvalueDiv.min = document.getElementById("rdmin").value;

    // pitch values
    const p_pvalueDiv = document.getElementById("ppValue");
    const p_ivalueDiv = document.getElementById("piValue");
    const p_dvalueDiv = document.getElementById("pdValue");

    p_pvalueDiv.min = document.getElementById("ppmin").value;
    p_pvalueDiv.max = document.getElementById("ppmax").value;
    p_ivalueDiv.max = document.getElementById("pimax").value;
    p_ivalueDiv.min = document.getElementById("pimin").value;
    p_dvalueDiv.max = document.getElementById("pdmax").value;
    p_dvalueDiv.min = document.getElementById("pdmin").value;

    // yaw values
    const y_pvalueDiv = document.getElementById("ypValue");
    const y_ivalueDiv = document.getElementById("yiValue");
    const y_dvalueDiv = document.getElementById("ydValue");

    y_pvalueDiv.max = document.getElementById("ypmax").value;
    y_pvalueDiv.min = document.getElementById("ypmin").value;
    y_ivalueDiv.max = document.getElementById("yimax").value;
    y_ivalueDiv.min = document.getElementById("yimin").value;
    y_dvalueDiv.max = document.getElementById("ydmax").value;
    y_dvalueDiv.min = document.getElementById("ydmin").value;

    document.getElementById('rpValue').step = (document.getElementById("rpmax").value - document.getElementById("rpmin").value)/100;
    document.getElementById('riValue').step = (document.getElementById("rimax").value - document.getElementById("rimin").value)/100;
    document.getElementById('rdValue').step = (document.getElementById("rdmax").value - document.getElementById("rdmin").value)/100;
    document.getElementById('ppValue').step = (document.getElementById("ppmax").value - document.getElementById("ppmin").value)/100;
    document.getElementById('piValue').step = (document.getElementById("pimax").value - document.getElementById("pimin").value)/100;
    document.getElementById('pdValue').step = (document.getElementById("pdmax").value - document.getElementById("pdmin").value)/100;
    document.getElementById('ypValue').step = (document.getElementById("ypmax").value - document.getElementById("ypmin").value)/100;
    document.getElementById('yiValue').step = (document.getElementById("yimax").value - document.getElementById("yimin").value)/100;
    document.getElementById('ydValue').step = (document.getElementById("ydmax").value - document.getElementById("ydmin").value)/100;
}


 function updatePIDs_text(){
    document.getElementById('rp_value_text').innerText = roll_P;
    document.getElementById('ri_value_text').innerText = roll_I;
    document.getElementById('rd_value_text').innerText = roll_D;
    document.getElementById('pp_value_text').innerText = pitch_P;
    document.getElementById('pi_value_text').innerText = pitch_I;
    document.getElementById('pd_value_text').innerText = pitch_D;
    document.getElementById('yp_value_text').innerText = yaw_P;
    document.getElementById('yi_value_text').innerText = yaw_I;
    document.getElementById('yd_value_text').innerText = yaw_D;
    document.getElementById('basethrottle_text').innerText = basethrottle;
}

let black_box_btn = document.getElementById("blackbox_btn");

function blackbox() {
    if (record_orientation === 0) {
        recording();
    } else{
        alert("Blackbox started, go to log tab to stop the recording!");
    }
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// //
// // // SETTINGS RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 

// You can now access the min and max values for roll, pitch, and yaw directly
const rollMaxInput = document.getElementById('roll-max');
const pitchMaxInput = document.getElementById('pitch-max');
const yawMaxInput = document.getElementById('yaw-max');
const speedMinInput = document.getElementById('speed-min');
const speedMaxInput = document.getElementById('speed-max');

// setting the values from the drone or default some values
function save_settings() {
    rollMax = rollMaxInput.value;
    pitchMax = pitchMaxInput.value;
    yawMax = yawMaxInput.value;
    speedMin = speedMinInput.value;
    speedMax = speedMaxInput.value;

    alert('Settings need to be Saved/Exported!')
    console.log('Settings need to be Saved/Exported!')
}

function import_settings(){
    rollMaxInput.value = rollMax;
    pitchMaxInput.value = pitchMax;
    yawMaxInput.value = yawMax;
    speedMinInput.value = speedMin;
    speedMaxInput.value = speedMax;

    console.log('Settings are imported!')        
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// //
// // // DRONE DISPLAY RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
    // Function to update the drone orientation
    function updateDroneOrientation() {
        //update drone actual orientation
        document.getElementById("roll").innerText = roll;
        document.getElementById("pitch").innerText = pitch;
        document.getElementById("yaw").innerText = yaw;
        document.getElementById("altitude").innerText = altitude;

        document.getElementById("roll_log").innerText = roll;
        document.getElementById("pitch_log").innerText = pitch;
        document.getElementById("yaw_log").innerText = yaw;
        document.getElementById("altitude_log").innerText = altitude;

        // Apply 3D transform to the drone block
        document.getElementById("drone").style.transform = `rotateX(${pitch}deg) rotateY(${roll}deg) rotateZ(${yaw}deg)`;
    }


    // Initialize the drone orientation
    updateDroneOrientation();



// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // MOTORS RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 

// for slider to control the motors speed
const speedSlider = document.getElementById('speed-slider');
const speedDisplay = document.getElementById('speed-display');

function update_min_max_speeds(){
    speedSlider.min = speedMin;
    speedSlider.max = speedMax;
    speedSlider.value = speedMin;
    speedDisplay.textContent = `Speed: ${speedMin}`;
}
update_min_max_speeds();

function reset_speed(){
    speedSlider.value=speedMin; 
    speedDisplay.textContent = `Speed: ${speedMin}`;
    // sending a min_speed commands to the drone over websocket
    motors_speed = speedMin;
    setMode(4); // 4 -> MOTOR_TEST
    motor_test();
}

// Event listener for slider input
speedSlider.addEventListener('input', () => {
    motors_speed = parseInt(speedSlider.value)
    speedDisplay.textContent = `Speed: ${motors_speed}`;
    // sending the speed value to the drone over websocket
    motor_test();
});

// for motor speed controlling code
let speed_control_state = 0;
function change_speed_control_state(this_element) {
    const speed_controller_block = document.getElementById('speed_controller_div');
    if (speed_control_state == 1) {
        speed_controller_block.style.display = 'none';
        speed_control_state = 0;
        this_element.innerText = "start motor test";
        reset_speed();
        setMode(0); // 0 -> STABLE
    }
    else if (confirm("Remove the props before doing it. Do you want to continue?")) {
        setMode(4); // 4 -> MOTOR_TEST
        speed_controller_block.style.display = 'flex';
        speed_control_state = 1;
        this_element.innerText = "stop motor test";
    } else {
        // Optional: Do something else if the user clicks "No"
        alert("Action canceled. Please remove the props first.");
    }
}


// Motors selection
const motorCalibDiv = document.getElementById("drone_info");
const propellers = motorCalibDiv.querySelectorAll(".propeller");
const arms = motorCalibDiv.querySelectorAll(".arm");

const modal = document.getElementById("propellerModal");
const overlay = document.getElementById("overlay");
const modalChoices = document.getElementById("modalChoices");

let selectedPropeller = null;

// show the motors and there selection status and there order in ['top-left', 'top-right', 'bottom-left', 'bottom-right']
function log_selected_motors(){
    // Prepare the formatted string for the alert
    const positions = ['top-left', 'top-right', 'bottom-left', 'bottom-right'];
    let statusMessage = '';

    positions.forEach((position, index) => {
        const statusText = motorsSelectionStatus[index] === 1 ? "selected" : "not selected";
        statusMessage += `${position} \t ESC ${motorsInOrder[index]} \t ${statusText}\n`;

    });


    statusMessage += "NOTE: \n";
    statusMessage += "1. To select/unselect motor click on the corresponing arm(rectangle)\n";
    statusMessage += "2. to change the motor number click on the corresponing motor(circle)\n";
    // Log the results
    console.log(statusMessage);
    // Show the alert with the formatted message
    alert(statusMessage);
}


// Show the modal with available choices
propellers.forEach(propeller => {
    propeller.addEventListener("click", () => {
        selectedPropeller = propeller;
        const currentValue = Number(propeller.textContent);

        // Clear old choices
        modalChoices.innerHTML = "";

        // Generate choices dynamically
        for (let i = 1; i <= 4; i++) {
            const choice = document.createElement("div");
            choice.innerHTML = `
                <label>
                    <input type="radio" name="propellerChoice" value="${i}" ${i === currentValue ? "checked" : ""}>
                    ${i}
                </label>
            `;
            modalChoices.appendChild(choice);
        }

        // Show modal
        modal.style.display = "block";
        overlay.style.display = "block";
    });
});


// Arms selection logic
const selectedArms = new Set();

document.querySelectorAll('.arm').forEach(arm => {
    arm.addEventListener('click', () => {
        const armName = arm.dataset.arm;

        if (selectedArms.has(armName)) {
            // Deselect the arm
            selectedArms.delete(armName);
            arm.classList.remove('selected');
        } else {
            // Select the arm
            selectedArms.add(armName);
            arm.classList.add('selected');
        }

        // Get motor selection status
        motorsSelectionStatus = Array.from(arms).map(arm => {
            return selectedArms.has(arm.dataset.arm) ? 1 : 0;
        });

        alert("motors in order " + motorsInOrder + "\nmotor selection status "+ motorsSelectionStatus);
    });
});


// Confirm selection
function confirm_selected_choice(){
    const selectedValue = document.querySelector('input[name="propellerChoice"]:checked');
    if (selectedValue) {
        const currentValue = Number(selectedPropeller.textContent);
        const newValue = Number(selectedValue.value);

        // Swap numbers if the new value is already assigned
        if (newValue !== currentValue) {
            const otherPropeller = Array.from(propellers).find(
                prop => Number(prop.textContent) === newValue
            );

            if (otherPropeller) {
                // Swap values
                otherPropeller.textContent = currentValue;
            }

            selectedPropeller.textContent = newValue;
        }

        // Hide modal
        modal.style.display = "none";
        overlay.style.display = "none";

        // update the motors_order
        motorsInOrder = Array.from(propellers).map(propeller =>
            Number(propeller.textContent)
        );
        console.log("motors in order " + motorsInOrder + "\nmotor selection status "+ motorsSelectionStatus);
        set_motors_order();
    } else {
        alert("Please select a value before confirming.");
    }
}

// Cancel selection
function cancel_selected_choice(){
    // Reset modal state
    modal.style.display = "none";
    overlay.style.display = "none";
    modalChoices.innerHTML = ""; // Clear choices to prevent duplicate elements
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // LOGGING RELATED THINGS
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
function show_store_data(){
    document.getElementById("store").classList.add('active_option');
    document.getElementById("load").classList.remove('active_option');

    document.getElementById("saveSection").style.display = "flex";
    document.getElementById("loadSection").style.display = "none";
}

function show_load_data(){
    document.getElementById("store").classList.remove('active_option');
    document.getElementById("load").classList.add('active_option');

    document.getElementById("saveSection").style.display = "none";
    document.getElementById("loadSection").style.display = "flex";
    
    const select = document.getElementById('dataSets');
    select.innerHTML = '';
    for (let i = 0; i < localStorage.length; i++) {
        const key = localStorage.key(i);
        const option = document.createElement('option');
        option.value = key;
        option.textContent = key;
        select.appendChild(option);
    }
}

function recording(){
    if(record_orientation === 0){
        console.log("starting the record");
        document.getElementById("recordBtn").innerText = "Finish Recording";
        document.getElementById("recordBtn").style.background = "#d32121";
        record_orientation = 1;

        black_box_btn.innerText = "Started recording... go to log tab";
        black_box_btn.style.backgroundColor = "red";

    }else if(record_orientation === 1){
        console.log("ending the record");
        document.getElementById("recordBtn").style.display = "none";
        document.getElementById("storeBtn").style.display = "flex";
        const timestamp = new Date().toISOString().replace(/[-T:.Z]/g, "_"); 
        document.getElementById("dataSetName").value = timestamp;
        
        record_orientation = 2;

        black_box_btn.innerText = "saving the file... go to log tab";
        black_box_btn.style.backgroundColor = "red";
    }
}


let rollData = [], pitchData = [], yawData = [], altitudeData = [], labels = [];
let error1Data = [], error2Data = [], error3Data = [], error4Data = [];
let pidOut1Data = [], pidOut2Data = [], pidOut3Data = [], pidOut4Data = [];
let motorSpeed1Data = [], motorSpeed2Data = [], motorSpeed3Data = [], motorSpeed4Data = [];
let timestampData = [];  // Store timestamps
let index = 0;
let maxPoints = 50;

const ctx = document.getElementById('rpyChart').getContext('2d');
const chart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: labels,
        datasets: [
            { label: 'Roll', borderColor: 'red', data: rollData, fill: false },
            { label: 'Pitch', borderColor: 'green', data: pitchData, fill: false },
            { label: 'Yaw', borderColor: 'blue', data: yawData, fill: false },
            { label: 'Altitude', borderColor: 'purple', data: altitudeData, fill: false },
            { label: 'Error1', borderColor: 'orange', data: error1Data, fill: false },
            { label: 'Error2', borderColor: 'brown', data: error2Data, fill: false },
            { label: 'Error3', borderColor: 'pink', data: error3Data, fill: false },
            { label: 'Error4', borderColor: 'gray', data: error4Data, fill: false },
            { label: 'PID Out 1', borderColor: 'cyan', data: pidOut1Data, fill: false },
            { label: 'PID Out 2', borderColor: 'magenta', data: pidOut2Data, fill: false },
            { label: 'PID Out 3', borderColor: 'yellow', data: pidOut3Data, fill: false },
            { label: 'PID Out 4', borderColor: 'lime', data: pidOut4Data, fill: false },
            { label: 'Motor Speed 1', borderColor: 'darkred', data: motorSpeed1Data, fill: false },
            { label: 'Motor Speed 2', borderColor: 'darkgreen', data: motorSpeed2Data, fill: false },
            { label: 'Motor Speed 3', borderColor: 'darkblue', data: motorSpeed3Data, fill: false },
            { label: 'Motor Speed 4', borderColor: 'darkpurple', data: motorSpeed4Data, fill: false }
        ]
    },
    options: { responsive: true }
});

function add_data_to_graph() {  
    rollData.push(roll);
    pitchData.push(pitch);
    yawData.push(yaw);
    altitudeData.push(altitude);
    error1Data.push(error1);
    error2Data.push(error2);
    error3Data.push(error3);
    error4Data.push(error4);
    pidOut1Data.push(pid_out1);
    pidOut2Data.push(pid_out2);
    pidOut3Data.push(pid_out3);
    pidOut4Data.push(pid_out4);
    motorSpeed1Data.push(motor_speed1);
    motorSpeed2Data.push(motor_speed2);
    motorSpeed3Data.push(motor_speed3);
    motorSpeed4Data.push(motor_speed4);
    timestampData.push(timestamp);  // Store timestamp

    labels.push(index++);
    updatePlot();
}

function updatePlot() {
    // if (rollData.length > maxPoints) {
    //     rollData.shift();
    //     pitchData.shift();
    //     yawData.shift();
    //     altitudeData.shift();
    //     error1Data.shift();
    //     error2Data.shift();
    //     error3Data.shift();
    //     error4Data.shift();
    //     pidOut1Data.shift();
    //     pidOut2Data.shift();
    //     pidOut3Data.shift();
    //     pidOut4Data.shift();
    //     motorSpeed1Data.shift();
    //     motorSpeed2Data.shift();
    //     motorSpeed3Data.shift();
    //     motorSpeed4Data.shift();
    //     timestampData.shift();  // Shift timestamps as well
    //     labels.shift();
    // }
    chart.update();
}

function storeData() {
    const name = document.getElementById('dataSetName').value.trim();
    if (!name) {
        alert('Please enter a dataset name.');
        return;
    }
    const data = { 
        timestampData, rollData, pitchData, yawData, altitudeData,
        error1Data, error2Data, error3Data, error4Data,
        pidOut1Data, pidOut2Data, pidOut3Data, pidOut4Data,
        motorSpeed1Data, motorSpeed2Data, motorSpeed3Data, motorSpeed4Data,
        labels, index 
    };
    localStorage.setItem(name, JSON.stringify(data));
    alert(`Data stored as "${name}"!`);
    clearData();
}

function clearData() {
    rollData = []; pitchData = []; yawData = []; altitudeData = []; labels = [];
    error1Data = []; error2Data = []; error3Data = []; error4Data = [];
    pidOut1Data = []; pidOut2Data = []; pidOut3Data = []; pidOut4Data = [];
    motorSpeed1Data = []; motorSpeed2Data = []; motorSpeed3Data = []; motorSpeed4Data = [];
    timestampData = [];  // Clear timestamps
    index = 0;
    chart.data.labels = labels;
    chart.data.datasets.forEach(dataset => dataset.data = []);
    chart.update();

    document.getElementById("storeBtn").style.display = "none";
    document.getElementById("recordBtn").style.display = "flex"; 
    document.getElementById("recordBtn").style.background = "rgb(63 169 58)";
    document.getElementById("recordBtn").innerText = "Start Recording";
    record_orientation = 0;

    black_box_btn.innerText = "start blackbox";
    black_box_btn.style.backgroundColor = "green";
}

function loadData() {
    const name = document.getElementById('dataSets').value;
    const storedData = JSON.parse(localStorage.getItem(name));
    if (storedData) {
        Object.assign({ rollData, pitchData, yawData, altitudeData, error1Data, error2Data, error3Data, error4Data, pidOut1Data, pidOut2Data, pidOut3Data, pidOut4Data, motorSpeed1Data, motorSpeed2Data, motorSpeed3Data, motorSpeed4Data, labels, index }, storedData);
        chart.data.labels = labels;
        chart.data.datasets.forEach((dataset, i) => dataset.data = Object.values(storedData)[i]);
        chart.update();
    } else {
        alert('No data found for the given name.');
    }
}

function downloadData() {
    const name = document.getElementById('dataSets').value;
    const storedData = JSON.parse(localStorage.getItem(name));

    if (!storedData) {
        alert('No data found for the given name.');
        return;
    }

    let csvContent = "data:text/csv;charset=utf-8,";

    // Extract keys as headers
    const headers = Object.keys(storedData).filter(key => Array.isArray(storedData[key]));
    csvContent += "Index," + headers.join(",") + "\n";

    // Find the max length of data arrays
    const rowCount = storedData[headers[0]].length;

    for (let i = 0; i < rowCount; i++) {
        let row = [i]; // Index column
        headers.forEach(header => {
            row.push(storedData[header][i]);
        });
        csvContent += row.join(",") + "\n";
    }

    const encodedUri = encodeURI(csvContent);
    const link = document.createElement("a");
    link.href = encodedUri;
    link.download = `${name}.csv`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}


function deleteData() {
    const name = document.getElementById('dataSets').value;

    if (!name) {
        alert("Please enter a dataset name to delete.");
        return;
    }

    if (localStorage.getItem(name)) {
        const confirmDelete = confirm(`Are you sure you want to delete the dataset "${name}"?`);
        
        if (confirmDelete) {
            localStorage.removeItem(name); // Remove the stored data from localStorage
            alert(`Data set "${name}" has been deleted.`);

            // Optionally, clear the chart after deletion
            chart.data.labels = [];
            chart.data.datasets.forEach(dataset => dataset.data = []);
            chart.update();
            const select = document.getElementById('dataSets');
            
            select.innerHTML = '';
            for (let i = 0; i < localStorage.length; i++) {
                const key = localStorage.key(i);
                const option = document.createElement('option');
                option.value = key;
                option.textContent = key;
                select.appendChild(option);
            }
        }
    } else {
        alert('No data found for the given name.');
    }
}


