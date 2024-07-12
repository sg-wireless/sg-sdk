

from machine import Timer
import utime, vtimers

print('-- init t1')
vt1 = Timer(-1)
vt1.init( period=3000, name="t1-periodic", mode = Timer.PERIODIC,
    callback=lambda t:print('T1-Expire'))

print('-- init t2')
vt2 = Timer(-1)
vt2.init( period=2000, name="t2-periodic", mode = Timer.PERIODIC,
    callback=lambda t:print('T2-Expire'))

print('-- init t3')
vt3 = Timer(-1)
vt3.init( period=1000, name="t3-one-shot", mode = Timer.ONE_SHOT,
    callback=lambda t:print('T3-Expire'))

print('-- init t4')
vt4 = Timer(-1)
vt4.init( period=2000, name="t4-periodic", mode = Timer.PERIODIC,
    callback=lambda t:print('T4-Expire'))

vtimers.stats()

print('-- sleep 3 seconds')
utime.sleep_ms(3000)

print(vt1.value())
print(vt2.value())
print(vt3.value())
print(vt4.value())

print('-- re-init t1')
vt1.init( period=1500, name="t1-NEW", mode = Timer.PERIODIC,
    callback=lambda t:print('T1-NEW-Expire'))

print('-- sleep 3 seconds')
utime.sleep_ms(3000)

print('-- deinit t2')
vt2.deinit()
vtimers.stats()

print('-- sleep 5 seconds')
utime.sleep_ms(5000)

print('-- deinit t1')
vt1.deinit()
vtimers.stats()
print('-- deinit t3')
vt3.deinit()
vtimers.stats()
print('-- deinit t4')
vt4.deinit()
vtimers.stats()

print('-- deinit t1')
vt1.deinit()
vtimers.stats()

print(vt1)
print(vt2)
print(vt3)
print(vt4)
