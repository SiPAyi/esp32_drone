// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// //
// // // settings related
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 

// You can now access the min and max values for roll, pitch, and yaw directly
    const rollMinInput = document.getElementById('roll-min');
    const rollMaxInput = document.getElementById('roll-max');
    const pitchMinInput = document.getElementById('pitch-min');
    const pitchMaxInput = document.getElementById('pitch-max');
    const yawMinInput = document.getElementById('yaw-min');
    const yawMaxInput = document.getElementById('yaw-max');
    const speedMinInput = document.getElementById('speed-min');
    const speedMaxInput = document.getElementById('speed-max');

// setting the values from the drone or default some values
    function save_settings() {
        rollMin = rollMinInput.value;
        rollMax = rollMaxInput.value;
        pitchMin = pitchMinInput.value;
        pitchMax = pitchMaxInput.value;
        yawMin = yawMinInput.value;
        yawMax = yawMaxInput.value;
        speedMin = speedMinInput.value;
        speedMax = speedMaxInput.value;

        alert('Settings need to be Saved!')
        console.log('Settings need to be Saved!')
    }

    function import_settings(){
        rollMinInput.value = rollMin;
        rollMaxInput.value = rollMax;
        pitchMinInput.value = pitchMin;
        pitchMaxInput.value = pitchMax;
        yawMinInput.value = yawMin;
        yawMaxInput.value = yawMax;
        speedMinInput.value = speedMin;
        speedMaxInput.value = speedMax;
    }



// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// //
// // // websocket related things
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
    let ip_address = "192.168.164.42";
    const ipDiv = document.getElementById("ip_address_span");
    const conn_status_div = document.getElementById("connection_status");
    ipDiv.innerText = ip_address;

    function changeIp() {
        ip_address = ipDiv.innerText;
        alert("IP Address changed to" + ip_address);
        ws.close();
        connectWebSocket();
    }


    let ws, prev_status = 0;
    function connectWebSocket() {
        ws = new WebSocket("ws://" + ip_address + ":81/");
        console.log("connecting to the websocket with address ws://" + ip_address + ":81/" + " type of ip_address " + typeof (ip_address))

        ws.onopen = function (event) {
            console.log("Connected to WebSocket server");
            conn_status_div.innerText = "Connected";
            conn_status_div.style.color = "Green";
            prev_status = 1;
        };

        ws.onmessage = function (event) {
            console.log("Server response: " + event.data);
            handleMessage(event.data);
        };
        
        ws.onerror = function (event) {
            console.error("WebSocket error observed:", event);
            // Attempt to reconnect after a short delay
            setTimeout(connectWebSocket, 2000);
            if (prev_status) {
                conn_status_div.innerText = "disconnected";
                conn_status_div.style.color = "red";
            } else {
                conn_status_div.innerText = "Not Connected";
                conn_status_div.style.color = "blue";
            }
        };

        ws.onclose = function (event) {
            console.error("WebSocket is closed:", event);
            conn_status_div.innerText = "disconnected";
            conn_status_div.style.color = "red";
        };
    }


    function send_data(msg){
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(msg);
            console.log("sent mesage: ", msg);
        } else {
            console.error("WebSocket connection is not open.");
            if (prev_status) {
                conn_status_div.innerText = "disconnected";
                conn_status_div.style.color = "red";
            } else {
                conn_status_div.innerText = "Not Connected";
                conn_status_div.style.color = "blue";
            }
        }
    }

    // Function to parse incoming messages
    function handleMessage(message) {
        try {
            const data = JSON.parse(message);
            const messageId = data.message_id;

            // Handle message based on message_id
            switch (messageId) {
                case 0: // 0(drone<-GCS): modes(0->standby, 1->motor_calib, 2->imu_calib, 3->guided, 4-> auto)
                    console.log("this step is only on drone side (drone<-GCS)");
                    break;
                case 1: // 1(drone<>GCS): get_PIDs
                    console.log("sending PID values");
                    send_data(msg);
                case 2: // 2(drone<>GCS): set_PIDs{save, proll, iroll, droll, ppitch, ipitch, dpitch, pyaw, iyaw, dyaw}
                    console.log("updating the PID values");
                    update_pids(msg.proll, msg.iroll, msg.droll, msg.ppitch, msg.ipitch, msg.dpitch, msg.pyaw, msg.iyaw, msg.dyaw)
                    break;
                case 3: // 3(drone<>GCS): get_motors_order
                    break;
                case 4: // 4(drone<-GCS): set_motors_order{top_left, top_right, bottom_left, bottom_right}
                    break;
                case 5: // 5(drone<-GCS): motor_calib{m1_state, m2_state, m3_state, m4_state, m1_speed, m2_speed, m3_speed, m4_speed}
                    break;
                case 6: // 6(drone<>GCS): get_settings
                    break;
                case 7: // 7(drone<>GCS): set_settings{max_roll, max_pitch, max_yaw, min_speed, max_speed}
                    break;
                case 8: // 8(drone<-GCS): imu_calib_command{step}
                    break;
                case 9: // 9(drone->GCS): imu_calib_response{step, response}
                    break;
                case 10: // 10(drone<-GCS): get_orientation
                    break;
                case 11: // 11(drone->GCS): drone_orientation{roll, pitch, yaw}
                    break;
                case 12: // 12(drone<-GCS): setpoints{roll, pitch, yaw}
                    break;
                default:
                    console.log(`Unknown Message ID: ${messageId}`);
            }
        } catch (err) {
            parsedData.textContent = `Error parsing message: ${err}`;
        }
    }

    // // Connect WebSocket when the page loads
    // window.onload = function () {
    //     connectWebSocket();
    // };


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// //
// // // drone display things
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
    // display related
    const roll_value_div = document.getElementById("roll");
    const pitch_value_div = document.getElementById("pitch");
    const yaw_value_div = document.getElementById("yaw");

    // Function to update the drone orientation
    function updateDroneOrientation() {
        //update drone actual orientation
        roll_value_div.innerText = roll;
        pitch_value_div.innerText = pitch;
        yaw_value_div.innerText = yaw;

        // Apply 3D transform to the drone block
        drone.style.transform = `rotateX(${pitch}deg) rotateY(${roll}deg) rotateZ(${yaw}deg)`;
    }


    // Initialize the drone orientation
    updateDroneOrientation();


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // calibration things
// // 
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
const esc_calib_steps = [
    { description: "Send maximum throttle signal to ESC.", visual: 'calibration_visuals/max_throttle.jpg', visual_type: 'image' },
    { description: "Connect the battery to the ESC.", visual: 'calibration_visuals/connect_battery.jpg', visual_type: 'image' },
    { description: "Send minimum throttle signal to finalize the calibration.", visual: 'calibration_visuals/min_throttle.jpg', visual_type: 'image' },
    { description: "Calibration complete! The ESC is ready for use.", visual: 'calibration_visuals/calib_complete.jpg', visual_type: 'image' }
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

let currentStep = 0;
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
});

escCalibBtn.addEventListener('click', () => {
    currentCalibration = esc_calib_steps;
    showCalibrationStep("ESC");

    escCalibBtn.classList.add('active');
    imuCalibBtn.classList.remove('active');
});


function showCalibrationStep(calib_items) {
    document.getElementById('calib').style.display = 'block';
    document.getElementById('calibrateBtn').style.display = 'block';
    document.getElementById('prevBtn').style.display = 'none';
    document.getElementById('nextBtn').style.display = 'none';
    document.getElementById('resetBtn').style.display = 'none';
    document.getElementById('step-description').innerHTML = "Let's start the " + calib_items + " calibration :)";
    document.getElementById('step-visual').innerHTML = `<img src="../calibration_visuals/calib_intro.jpg" alt="Step Visual"/>`; 
    document.getElementById('step-number').innerHTML = '';
    
    // Reset the current step
    currentStep = 0;
}

// Start button logic
document.getElementById('calibrateBtn').addEventListener('click', function() {
    showStep(currentStep);
    document.getElementById('calibrateBtn').style.display = 'none';
    document.getElementById('prevBtn').style.display = 'inline-block';
    document.getElementById('nextBtn').style.display = 'inline-block';
    document.getElementById('resetBtn').style.display = 'inline-block';
});

// Previous button logic
document.getElementById('prevBtn').addEventListener('click', function() {
    if (currentStep > 0) {
        currentStep--;
        showStep(currentStep);
    }
    
    // Re-enable the 'Next' button when moving back from the last step
    if (currentStep < currentCalibration.length - 1) {
        document.getElementById('nextBtn').disabled = false;
    }   

    // Enable the 'Previous' button at the beginning (if needed)
    if (currentStep > 0) {
        document.getElementById('prevBtn').disabled = false;
    }
});

// Next button logic
document.getElementById('nextBtn').addEventListener('click', function(event) {
    if (currentStep < currentCalibration.length - 1) {
        currentStep++;
        showStep(currentStep);
    }

    // Disable the 'Next' button on the last step
    if (currentStep === currentCalibration.length - 1) {
        document.getElementById('nextBtn').disabled = true;
    }

    // Re-enable the 'Previous' button if not on the first step
    if (currentStep > 0) {
        document.getElementById('prevBtn').disabled = false;
    }
});



// Reset button logic
document.getElementById('resetBtn').addEventListener('click', function() {
    document.getElementById('nextBtn').disabled = false;
    document.getElementById('prevBtn').disabled = false;
    currentStep = 0;
    showStep(currentStep);
});

function showStep(stepIndex) {
    const step = currentCalibration[stepIndex];
    document.getElementById('step-description').innerHTML = step.description;
    document.getElementById('step-number').innerHTML = `Step ${stepIndex + 1}`;
    if (step.visual_type === 'image') {
        document.getElementById('step-visual').innerHTML = `<img src="../${step.visual}" alt="Step Visual" width="300" />`;
    } else if (step.visual_type === 'video') {
        document.getElementById('step-visual').innerHTML = `<video autoplay loop><source src="../${step.visual}" type="video/webm"></video>`;
    }
    // Update the progress bar
    progressBar.style.width = (currentStep + 1) / currentCalibration.length * 100 + '%';
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // pid related things
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
    }

    function update_pids(r_pValue, r_iValue, r_dValue, p_pValue, p_iValue, p_dValue, y_pValue, y_iValue, y_dValue){
        document.getElementById('rp_value_text').innerText = r_pValue;
        document.getElementById('ri_value_text').innerText = r_iValue;
        document.getElementById('rd_value_text').innerText = r_dValue;
        document.getElementById('pp_value_text').innerText = p_pValue;
        document.getElementById('pi_value_text').innerText = p_iValue;
        document.getElementById('pd_value_text').innerText = p_dValue;
        document.getElementById('yp_value_text').innerText = y_pValue;
        document.getElementById('yi_value_text').innerText = y_iValue;
        document.getElementById('yd_value_text').innerText = y_dValue;  
    }

    function send_pids(save_or_not) {
        // roll values
        const r_pValue = parseFloat(document.getElementById('rpValue').value) || 0;
        const r_iValue = parseFloat(document.getElementById('riValue').value) || 0;
        const r_dValue = parseFloat(document.getElementById('rdValue').value) || 0;
        const p_pValue = parseFloat(document.getElementById('ppValue').value) || 0;
        const p_iValue = parseFloat(document.getElementById('piValue').value) || 0;
        const p_dValue = parseFloat(document.getElementById('pdValue').value) || 0; 
        const y_pValue = parseFloat(document.getElementById('ypValue').value) || 0;
        const y_iValue = parseFloat(document.getElementById('yiValue').value) || 0;
        const y_dValue = parseFloat(document.getElementById('ydValue').value) || 0;

        document.getElementById('rp_value_text').innerText = r_pValue;
        document.getElementById('ri_value_text').innerText = r_iValue;
        document.getElementById('rd_value_text').innerText = r_dValue;
        document.getElementById('pp_value_text').innerText = p_pValue;
        document.getElementById('pi_value_text').innerText = p_iValue;
        document.getElementById('pd_value_text').innerText = p_dValue;
        document.getElementById('yp_value_text').innerText = y_pValue;
        document.getElementById('yi_value_text').innerText = y_iValue;
        document.getElementById('yd_value_text').innerText = y_dValue;


        if (save_or_not === 1){
            console.log('saving the PID values');
            alert('saving the PID values');
        }
        
        const pidValues = {
            id: 2,
            save: save_or_not,
            proll: r_pValue,
            iroll: r_iValue,
            droll: r_dValue,
            ppitch: p_pValue,
            ipitch: p_iValue,
            dpitch: p_dValue,
            pyaw: y_pValue,
            iyaw: y_iValue,
            dyaw: y_dValue,
          };

        handleMessage(JSON.stringify(pidValues));
    }


// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // blocks hiding and unhiding things
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
    function show_or_hide_block(div_id) {
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

    }

// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // joy controller things
// //
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
    function setupJoystick(containerId, knobId, outputId) {
        const container = document.getElementById(containerId);
        const knob = document.getElementById(knobId);
        const output = document.getElementById(outputId);

        const containerRadius = container.offsetWidth / 2;
        const knobRadius = knob.offsetWidth / 2;
        const maxDistance = containerRadius - knobRadius;

        let isDragging = false;

        container.addEventListener("mousedown", (e) => {
            isDragging = true;
            moveKnob(e.clientX, e.clientY);
        });

        container.addEventListener("touchstart", (e) => {
            isDragging = true;
            const touch = e.touches[0];
            moveKnob(touch.clientX, touch.clientY);
        });

        document.addEventListener("mousemove", (e) => {
            if (isDragging) moveKnob(e.clientX, e.clientY);
        });

        document.addEventListener("touchmove", (e) => {
            if (isDragging) {
                const touch = e.touches[0];
                moveKnob(touch.clientX, touch.clientY);
            }
        });

        document.addEventListener("mouseup", () => {
            if (isDragging) resetKnob();
            isDragging = false;
        });

        document.addEventListener("touchend", () => {
            if (isDragging) resetKnob();
            isDragging = false;
        });

        function moveKnob(clientX, clientY) {
            const rect = container.getBoundingClientRect();
            const centerX = rect.left + containerRadius;
            const centerY = rect.top + containerRadius;

            let dx = clientX - centerX;
            let dy = clientY - centerY;

            const isRotated = window.matchMedia("(max-width: 450px)").matches;

            if (isRotated) {
                [dx, dy] = [dy, -dx];
            }

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

            output.textContent = `X: ${normalizedX}, Y: ${normalizedY}`;
        }

        function resetKnob() {
            knob.style.left = `${containerRadius}px`;
            knob.style.top = `${containerRadius}px`;
            output.textContent = `X: 0, Y: 0`;
        }
    }

    // Setup fullscreen functionality
    const joyBlock = document.getElementById("joy");
    const enterFullscreenBtn = document.getElementById("enterFullscreen");
    const exitFullscreenBtn = document.getElementById("exitFullscreen");


    enterFullscreenBtn.addEventListener("click", () => {
        if (joyBlock.requestFullscreen) {
            joyBlock.requestFullscreen();
        } else if (joyBlock.webkitRequestFullscreen) { // Safari
            joyBlock.webkitRequestFullscreen();
        } else if (joyBlock.msRequestFullscreen) { // IE/Edge
            joyBlock.msRequestFullscreen();
        }
        joyBlock.style.color = "white";
        enterFullscreenBtn.style.display = "none";
        exitFullscreenBtn.style.display = "inline-block";
    });

    exitFullscreenBtn.addEventListener("click", () => {
        if (document.exitFullscreen) {
            document.exitFullscreen();
        } else if (document.webkitExitFullscreen) { // Safari
            document.webkitExitFullscreen();
        } else if (document.msExitFullscreen) { // IE/Edge
            document.msExitFullscreen();
        }
        joyBlock.style.color = "black";
        enterFullscreenBtn.style.display = "inline-block";
        exitFullscreenBtn.style.display = "none";
    });

    setupJoystick("joystick1", "knob1", "output1");
    setupJoystick("joystick2", "knob2", "output2");


    
// // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// // 
// // // Motors related
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
        // need send a min_speed commands to the drone over websocket
    }

// Event listener for slider input
    speedSlider.addEventListener('input', () => {
        sliderValue = parseInt(speedSlider.value)
        speedDisplay.textContent = `Speed: ${sliderValue}`;
        // need send the speed value to the drone over websocket
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
        }
        else if (confirm("Remove the props before doing it. Do you want to continue?")) {
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
    // Get motor numbers in order
    const motorsInOrder = Array.from(propellers).map(propeller =>
        Number(propeller.textContent)
    );

    // Get motor selection status
    const motorsSelectionStatus = Array.from(arms).map(arm => {
        return selectedArms.has(arm.dataset.arm) ? 1 : 0;
    });

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
        });
    });


// Function to get selected arms (can be used anywhere)
    function getSelectedArms() {
        return Array.from(selectedArms);
    }

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