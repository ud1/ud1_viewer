#include <iostream>

#include "d_tcpclient.hpp"
#include <set>

SObj rect(const V3 &center, const V3 &t1, const V3 &t2, uint32_t color, const V3 &lightPos, const V3 &gridShift)
{
    SObj res;

    res["type"] = "rect";
    res["gridShift"] = gridShift;
    res["c"] = color;
    res["p"] = center;
    res["ht1"] = t1;
    res["ht2"] = t2;
    res["lp"] = lightPos;

    return res;
}

SObj sphere(const V3 &pos, float r, uint32_t color, const V3 &lightPos)
{
    SObj res;

    res["type"] = "sphere";
    res["p"] = pos;
    res["r"] = r;
    res["c"] = color;
    res["lp"] = lightPos;

    return res;
}

Obj ball(const V3 &ballPos)
{
    double x = ballPos.x;
    double y = ballPos.y;
    double z = ballPos.z;
    double r = 1;
    V3 lightPos = V3(16, 18, 10);

    Obj obj;
    obj.type = "ball";
    obj.subObjs["sph"] = sphere(V3(x, y, z), r, 0xffff00ffu, lightPos);

    obj.subObjs["disk"]["type"] = "disk";
    obj.subObjs["disk"]["p"] = V3(x, y, 0);
    obj.subObjs["disk"]["n"] = V3(0, 0, 1.5);
    obj.subObjs["disk"]["r"] = r;
    obj.subObjs["disk"]["c"] = 0xffff00ffu;

    obj.subObjs["l"]["type"] = "line3d";
    obj.subObjs["l"]["p1"] = V3(x, y, z);
    obj.subObjs["l"]["p2"] = V3(x, y, 0);
    obj.subObjs["l"]["c"] = 0xffff00ffu;

    return obj;
}

std::vector<Obj> buildField()
{
    std::vector<Obj> result;

    double L = 18;
    double W = 16;

    V3 lightPos = V3(W, L, 10);

    {
        Obj obj;
        obj.type = "field3d";
        obj.props["minP"] = P(0, 0);
        obj.props["maxP"] = P(W * 2, L * 2);
        obj.props["hMin"] = 0.0;
        obj.props["hMax"] = 10.0;
        obj.props["cellSize"] = 1.0;

        result.push_back(obj);
    }

    {
        Obj obj;
        obj.type = "static";
        obj.subObjs["floor"] = rect(
                V3(W, L, 0),
                V3(W, 0, 0),
                V3(0, L, 0),
                0xe57f00ffu,
                lightPos,
                V3(0, 0, 0.5)
        );

        obj.subObjs["floorDisk"]["type"] = "disk";
        obj.subObjs["floorDisk"]["p"] = V3(W, L, 0);
        obj.subObjs["floorDisk"]["n"] = V3(0, 0, 1);
        obj.subObjs["floorDisk"]["r"] = 0.2;
        obj.subObjs["floorDisk"]["c"] = 0x77ffffffu;

        result.push_back(obj);
    }

    return result;
}

void sendField(TcpClient &client)
{
    std::vector<Obj> result = buildField();
    for (const Obj &o : result)
        client.sendObj(o);
}

void sendNewTick(TcpClient &client, uint32_t tick)
{
    Obj obj;
    obj.type = "tick";
    obj.props["num"] = tick;
    client.sendObj(obj);
}

using namespace std::chrono_literals;

int main() {

    TcpClient client;
    client.run();

    sendField(client);

    bool pause = true;

    V3 pos = V3(5, 5, 3);

    auto step = [&](int tick){
        sendNewTick(client, tick);
        client.sendObj(ball(pos));
        pos += V3(0.1, 0.1, 0);
    };

    bool running = true;
    for (int tick = 0; tick < 1000 && running; )
    {
        std::set<uint32_t > pressed;
        Obj msg;
        while (client.inMsgs.tryPop(msg, pause ? 16ms : 0ms))
        {
            //LOG("GOT " << msg.type);
            if (msg.type.empty())
            {
                running = false;
                break;
            }

            if (msg.type == "keyPress")
            {
                if (msg.getIntProp("key") == KEYS::PAUSE)
                {
                    pause = !pause;
                }

                pressed.insert(msg.getIntProp("key"));
            }
        }

        if (!pause || pressed.count(KEYS::RIGHT))
        {
            ++tick;
            step(tick);
            usleep(16000);
        }
    }
    return 0;
} 
