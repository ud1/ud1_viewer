#define ENABLE_LOGGING 1
#include "myutils.hpp"

#include "d_tcpclient.hpp"

int main() {

    TcpClient tcpClient;
    tcpClient.run();

    int i = 0;

    Obj obj;
    obj.type = "fieldSize";
    obj.props["w"] = 200u;
    obj.props["h"] = 200u;
    tcpClient.sendObj(obj);

    while(true)
    {
        Obj obj = tcpClient.inMsgs.pop();
        if (obj.type.empty())
            break;
        LOG("O " << obj);

        {
            Obj obj;
            obj.type = "tick";
            obj.props["num"] = 123u + i;
            tcpClient.sendObj(obj);

            obj.type = "someObj";
            obj.props["i"] = 234u;

            obj.subObjs["subObj1"]["type"] = "circumference";
            obj.subObjs["subObj1"]["p"] = P(50 + i, 50 + i);
            obj.subObjs["subObj1"]["r"] = 10.0;
            obj.subObjs["subObj1"]["c"] = 0xff0000ffu;

            obj.subObjs["subObj2"]["type"] = "line";
            obj.subObjs["subObj2"]["p1"] = P(250 + i, 50);
            obj.subObjs["subObj2"]["p2"] = P(150, 50);
            obj.subObjs["subObj2"]["p3"] = P(80, 150);
            obj.subObjs["subObj2"]["c"] = 0xffff00ffu;

            obj.subObjs["subObj3"]["type"] = "poly";
            obj.subObjs["subObj3"]["p1"] = P(150, 150);
            obj.subObjs["subObj3"]["p2"] = P(150, 250);
            obj.subObjs["subObj3"]["p3"] = P(80, 230);
            obj.subObjs["subObj3"]["c"] = 0x00ff00ffu;

            tcpClient.sendObj(obj);

            LOG("WRITE TICK " << i);
            ++i;
        }

        {
            Obj obj;
            obj.type = "log";
            obj.props["text"] = "Some text";
            tcpClient.sendObj(obj);
        }
    }
    return 0;
}
