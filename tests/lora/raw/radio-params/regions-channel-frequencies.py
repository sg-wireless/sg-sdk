import lora

lora.mode(lora._mode.RAW)
lora.radio_params(region=lora._region.REGION_EU868)

print('-' * 30)
for ch in [ 868.10, 868.30, 868.50, 864.10, 864.30,
            864.50, 868.10, 868.30, 868.50 ]:
    print(f'LoRaWAN >> EU868 >> #ch {ch:.2f}')
    lora.radio_params(freq_mhz=ch)
print('-' * 30)
for ch in [ 865.20, 865.50, 865.80, 866.10, 866.40, 866.70, 867, 868 ]:
    print(f'RFWireless-World >> EU868 >> #ch {ch:.2f}')
    lora.radio_params(freq_mhz=ch)

lora.radio_params(region=lora._region.REGION_US915)
print('-' * 30)
for ch in list(range(902300000, 914910000, 200000)) + \
          list(range(903000000, 914210000, 1600000)) + \
          list(range(923300000, 927510000, 600000)):
    print(f'LoRaWAN >> US915 >> #ch {ch/1000000:.2f}')
    lora.radio_params(frequency=ch)

print('-' * 30)
for ch in [ 903.08, 905.24, 907.40, 909.56, 911.72, 913.88,
            916.04, 918.20, 920.36, 922.52, 924.68, 926.84, 915 ]:
    print(f'RFWireless-World >> US915 >> #ch {ch:.2f}')
    lora.radio_params(freq_mhz=ch)

lora.radio_params(region=lora._region.REGION_CN779)
print('-' * 30)
for ch in [ 779.5, 779.7, 779.9, 779.5, 780.5, 780.7, 780.9 ]:
    print(f'LoRaWAN >> CN779 >> #ch {ch:.2f}')
    lora.radio_params(freq_mhz=ch)
