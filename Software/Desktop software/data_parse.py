import serial
import struct
import time
import os
CHUNK_SIZE = 1280
FIFO_AMOUNT = 423
STRUCT_FORMAT = f"<q{FIFO_AMOUNT * 3}sBH"

binary_file = "check.bin"
file_green = "m_green.csv"
file_red = "m_red.csv"
file_IR = "m_ir.csv"
file_EDA = "m_eda.csv"
file_steps = "m_steps.csv"
file_other = "m_other.csv"

def parse_container(raw_data, index):
    try:
        time_sec, fifo_raw, flags, steps = struct.unpack(STRUCT_FORMAT, raw_data)
        
        print(f"\n[Batch {index}] Time: {time_sec} | Steps: {steps} | Flags: {hex(flags)}")
        
        # Parse first 2 FIFO samples for verification
        # Sample = 3 bytes: [LB, MB, HB]
        for i in range(423):
            offset = i * 3
            lb = fifo_raw[offset]
            mb = fifo_raw[offset+1]
            hb = fifo_raw[offset+2]
            
            adc_val = (hb << 12) | (mb << 4) | (lb >> 4)
            marker = lb & 0x07
            print(f"  S{i}: ADC={adc_val}, Marker={marker}")
            
    except struct.error as e:
        print(f"Unpack error: {e}")

def main():
    #loading and parsing the file as described
    raw_file = open(binary_file, "rb")
    file_size= os.path.getsize(binary_file)
    if (file_size%CHUNK_SIZE==0):
        #proceed normally
        print("Processing file of size:", file_size, "bytes")
        i=0
        while (i<file_size):
            byte_chunk = raw_file.read(1280)
            parse_container(byte_chunk,i)
            i+=CHUNK_SIZE
    else:
        print("Error, file size not divisible by chunk size", file_size)
    
main()