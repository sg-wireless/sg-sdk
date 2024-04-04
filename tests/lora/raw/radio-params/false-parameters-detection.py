
import lora

lora.initialize()

lora.mode(lora._mode.RAW)

regions = [
    lora._region.REGION_AS923,
    lora._region.REGION_AU915,
    lora._region.REGION_CN470,
    lora._region.REGION_CN779,
    lora._region.REGION_EU433,
    lora._region.REGION_EU868,
    lora._region.REGION_IN865,
    lora._region.REGION_KR920,
    lora._region.REGION_RU864,
    lora._region.REGION_US915
]

print('-- reset all parameters -- region EU868 ----------------')
lora.radio_params( reset_all=True )

print('-- lora stats')
lora.stats()

print('-- detect false region ---------------------------------')
lora.radio_params(region=44)

print('-- detect false frequency ------------------------------')
lora.radio_params(frequency=433000000)

print('-- detect false frequency with a given region ----------')
lora.radio_params(region=lora._region.REGION_AS923, frequency=433000000)

print('-- correct frequency with a given region ---------------')
lora.radio_params(region=lora._region.REGION_AS923, frequency=923000000)
lora.stats()

print('-- reset all parameters -- region EU868 ----------------')
lora.radio_params( reset_all=True )
lora.stats()

print('-- false spreading factor ------------------------------')
print('sf =  1 --> ', end=''); lora.radio_params(sf=1);  print('')
print('sf =  2 --> ', end=''); lora.radio_params(sf=2);  print('')
print('sf =  3 --> ', end=''); lora.radio_params(sf=3);  print('')
print('sf =  4 --> ', end=''); lora.radio_params(sf=4);  print('')
print('sf =  5 --> ', end=''); lora.radio_params(sf=5);  print('')
print('sf =  6 --> ', end=''); lora.radio_params(sf=6);  print('')
print('sf =  7 --> ', end=''); lora.radio_params(sf=7);  print('')
print('sf =  8 --> ', end=''); lora.radio_params(sf=8);  print('')
print('sf =  9 --> ', end=''); lora.radio_params(sf=9);  print('')
print('sf = 10 --> ', end=''); lora.radio_params(sf=10); print('')
print('sf = 11 --> ', end=''); lora.radio_params(sf=11); print('')
print('sf = 12 --> ', end=''); lora.radio_params(sf=12); print('')
print('sf = 13 --> ', end=''); lora.radio_params(sf=13); print('')
print('sf = 14 --> ', end=''); lora.radio_params(sf=14); print('')
lora.stats()

print('-- false bandwidth -------------------------------------')
print('bandwidth = lora._bw.BW_125KHZ --> ', end='')
lora.radio_params(bandwidth = lora._bw.BW_125KHZ)
print('')
print('bandwidth = lora._bw.BW_250KHZ --> ', end='')
lora.radio_params(bandwidth = lora._bw.BW_250KHZ)
print('')
print('bandwidth = lora._bw.BW_500KHZ --> ', end='')
lora.radio_params(bandwidth = lora._bw.BW_500KHZ)
print('')
print('bandwidth = 750 --> ', end='')
lora.radio_params(bandwidth = 750)
print('')
print('bandwidth = 100 --> ', end='')
lora.radio_params(bandwidth = 100)
print('')
lora.stats()

print('-- detect false region ---------------------------------')
lora.deinit()
