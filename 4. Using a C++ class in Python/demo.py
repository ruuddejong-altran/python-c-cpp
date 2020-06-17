from traffic_light import Light, TrafficLight


def monitor(tl):
    print(f"""[Python] State: {tl.state}, ({', '.join('{n}: {v}' for n, v in zip(tl.LightNames, tl.LightPattern))})""")



if __name__ == '__main__':
    tl = TrafficLight(Light)
    tl.Close()
    tl.Open()
    tl.Close()
    tl.Warning()
    tl.Off()
