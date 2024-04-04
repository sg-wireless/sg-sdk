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

def get_class_const_name(__class, __const):
    for k,v in __class.__dict__.items():
        if v == __const:
            return k
    return 'unknown'

for region in regions:
    print('--- region [ {} ] ------------------------------'.format(
        get_class_const_name(lora._region, region)))
    lora.radio_params( reset_all=True )
    lora.radio_params( region=region )
    lora.stats()

lora.deinit()
