import pandas as pd

# Gán đường dẫn file vào biến
input_file = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/raw/harbor/dataset_for_RSSI_harbor.xlsx'

# Read data from Excel file
df = pd.read_excel(input_file, sheet_name='Sheet1')

# List of gateways
gateways = ['GW1', 'GW2', 'GW3', 'GW4', 'GW5']

# Define the mapping dictionary
map_node = {
    '886198c0a7efee91': 'Node_A',
    '88c783b832d92380': 'Node_B',
    'bbc20572986ac467': 'Node_C'
}

# Normalize the node_id column: remove single quotes and whitespace
df['node_id'] = df['node_id'].str.strip().str.replace("'", "")

# Replace node_id values based on map_node
df['node_id'] = df['node_id'].map(map_node).fillna(df['node_id'])

# Create an empty list to store results
reshaped_data = []

# Convert the 'timestamp' column to datetime format
df['timestamp'] = pd.to_datetime(df['timestamp'])

# Process each row in the original data
for _, row in df.iterrows():
    timestamp = row['timestamp']
    latitude = row['latitude']
    longitude = row['longitude']
    node_id = row['node_id']
    
    # Check if location data exists
    # has_location = (latitude != 0 and longitude != 0)
    
    # Process each gateway
    for gw in gateways:
        rssi_col = f'RSSI_{gw}'
        snr_col = f'SNR_{gw}'
        
        # Check if signal data exists
        has_signal = (row[rssi_col] != 0 or row[snr_col] != 0)
        
        # Only append if there is signal data or location data
        # if has_signal and has_location: 
        reshaped_data.append({
            'timestamp': timestamp,
            'node_id': node_id,
            'gateway': gw,
            'rssi': row[rssi_col],
            'snr': row[snr_col],
            'latitude': latitude,
            'longitude': longitude
        })


# Create a DataFrame from the processed list
result_df = pd.DataFrame(reshaped_data)

# Rearrange the columns in the desired order
result_df = result_df[['timestamp', 'node_id', 'gateway', 'rssi', 'snr', 'latitude', 'longitude']]

# Define the output file path
output_file = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/processed/reshaped_data.xlsx'

# Save the result to a new Excel file
result_df.to_excel(output_file, index=False)

print(f"Completed. File saved to: {output_file}")