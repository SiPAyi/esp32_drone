import asyncio
import websockets
import json
import numpy as np

# Store the last 2000 data points
buffer_size = 2000
data_buffer = []

async def analyze_data():
    """ Process data and suggest PID tuning """
    if len(data_buffer) < 100:  # Wait until enough data is collected
        return {"status": "Collecting data..."}

    # Convert to NumPy array
    data_array = np.array(data_buffer)

    # Extract roll errors (Assuming last column is roll error)
    roll_error = data_array[:, 1]  # Adjust index based on actual data

    # Compute standard deviation (std dev measures oscillations)
    std_dev = np.std(roll_error)

    # Compute mean (to check for steady-state error)
    mean_error = np.mean(roll_error)

    # Compute derivative (rate of change of error)
    derivative = np.gradient(roll_error)

    # Check if derivative is too high (indicates aggressive oscillations)
    derivative_std = np.std(derivative)

    # Suggest PID tuning based on analysis
    suggestion = {
        "status": "Analyzed",
        "P": "Increase" if std_dev > 10 else "Decrease",
        "I": "Increase" if abs(mean_error) > 5 else "Decrease",
        "D": "Increase" if derivative_std > 5 else "Decrease"
    }

    return suggestion

async def handle_client(websocket, path):
    global data_buffer
    print("Client connected")

    try:
        async for message in websocket:
            data = json.loads(message)
            data_buffer.append([
                data['timestamp'], data['roll_error'], data['pitch_error'], data['yaw_error']
            ])
            if len(data_buffer) > buffer_size:
                data_buffer.pop(0)

            # Analyze and send back PID suggestion
            suggestion = await analyze_data()
            await websocket.send(json.dumps(suggestion))

    except websockets.exceptions.ConnectionClosed:
        print("Client disconnected")

async def main():
    async with websockets.serve(handle_client, "localhost", 8765):
        await asyncio.Future()  # Keep server running

if __name__ == "__main__":
    asyncio.run(main())
