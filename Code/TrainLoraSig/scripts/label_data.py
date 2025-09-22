import pandas as pd
import numpy as np

# Read the Excel file
input_file = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/processed/cleaned_lora_with_trace.xlsx'
df = pd.read_excel(input_file, sheet_name='cleaned_robust')

# --- Convert timestamp to datetime ---
df['timestamp'] = pd.to_datetime(df['timestamp'])

# --- High-performance: Compute quantiles by group and merge back ---
# Calculate 20th and 80th percentiles for each (node_id, gateway) group
quantiles = df.groupby(['node_id', 'gateway']).agg(
    q20_rssi=('rssi', lambda x: x.quantile(0.2)),
    q80_rssi=('rssi', lambda x: x.quantile(0.8)),
    q20_snr=('snr', lambda x: x.quantile(0.2)),
    q80_snr=('snr', lambda x: x.quantile(0.8))
).reset_index()

# Merge the quantiles back into the original dataframe
df = df.merge(quantiles, on=['node_id', 'gateway'], how='left')

# Quickly assign quality labels using np.select
conditions = [
    # 'good' if both RSSI and SNR >= 80th percentile
    (df['rssi'] >= df['q80_rssi']) & (df['snr'] >= df['q80_snr']),
    # 'poor' if either RSSI or SNR <= 20th percentile
    (df['rssi'] <= df['q20_rssi']) | (df['snr'] <= df['q20_snr'])
]
choices = ['good', 'poor']
# Assign 'fair' as default for all other rows
df['quality_quantile'] = np.select(conditions, choices, default='fair')

# --- Select the best gateway efficiently ---
# Sort by timestamp, node_id ascending, then SNR descending, RSSI descending
df = df.sort_values(['timestamp', 'node_id', 'snr', 'rssi'], ascending=[True, True, False, False])

# Initialize the column as False
df['is_best_gateway'] = False
# Keep only the first row per (timestamp, node_id) after sorting → best gateway
best_idx = df.drop_duplicates(['timestamp', 'node_id'], keep='first').index
df.loc[best_idx, 'is_best_gateway'] = True

# Save the labeled dataframe to a new Excel file
output_file = 'C:/Users/user/Documents/CapstoneProject/Code/TrainLoraSig/data/labels/lora_labeled.xlsx'
df.to_excel(output_file, sheet_name='labeled_data', index=False)

print(f"Completed. File saved")
