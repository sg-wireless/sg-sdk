
# assume this test case is under development and to not interrupt
# the already running test flows, the script will tell the test system to
# skip this test case by printing 'SKIP' as an output of this script

still_under_development = True

if still_under_development:
    print('SKIP')   # inform the test engine to skip this test case
else:

    print('demonstrating single micropython test case')

    for counter in range(10):
        print('counting up {}'.format(counter))

    print('end test case')
