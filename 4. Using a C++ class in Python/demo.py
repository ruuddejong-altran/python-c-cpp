import traffic
from datetime import datetime
import time


def monitor(tl):
    print(f"{datetime.now().time()} {tl.state.name} ({', '.join(f'{n}: {s.name}' for n, s in zip(tl.names, tl.pattern))})")


def force_closed(tl):
    if (tl.state == tl.State.Open):
        tl.MoveTo(tl.State.Closed)


if __name__ == '__main__':
    print("Testing light")
    l = traffic.Light()
    print(f"New light should be off. Actual state is {l.state.name}")
    l.state = l.On
    print(f"Setting light on. Actual state is {l.state.name}")
    l.state = l.Flashing
    print(f"Setting light flashing. Actual state is {l.state.name}")
    print()
    print("Testing traffic light")
    t = traffic.TrafficLight()
    t.AddCallback(monitor)
    t.AddCallback(force_closed)
    print("Moving to Closed")
    t.MoveTo(t.State.Closed)
    print("Moving to Open (expect return to Closed)")
    t.MoveTo(t.State.Open)
    while t.in_transition:
        time.sleep(0.1)

