#include "basic/pregel-dev.h"
#include <set>
using namespace std;

//OWCTY

//input format:
//vid \t color sccTag in_num in1 in2 ... out_num out1 out2 ...
//-1 \t nextColorToAssign

//output format:
//vid \t color sccTag in_num in1 in2 ... out_num out1 out2 ...
//-1 \t nextColorToAssign

struct OWCTYValue_scc {
    int color;
    //color=-1, means trivial SCC
    //color=-2, means not assigned
    //otherwise, color>=0
    int sccTag;
    vector<VertexID> in_edges;
    vector<VertexID> out_edges;
};

ibinstream& operator<<(ibinstream& m, const OWCTYValue_scc& v)
{
    m << v.color;
    m << v.sccTag;
    m << v.in_edges;
    m << v.out_edges;
    return m;
}

obinstream& operator>>(obinstream& m, OWCTYValue_scc& v)
{
    m >> v.color;
    m >> v.sccTag;
    m >> v.in_edges;
    m >> v.out_edges;
    return m;
}

//====================================

class OWCTYVertex_scc : public Vertex<VertexID, OWCTYValue_scc, VertexID> {
public:
    void bcast_to_in_nbs(VertexID msg)
    {
        vector<VertexID>& nbs = value().in_edges;
        for (int i = 0; i < nbs.size(); i++) {
            send_message(nbs[i], msg);
        }
    }

    void bcast_to_out_nbs(VertexID msg)
    {
        vector<VertexID>& nbs = value().out_edges;
        for (int i = 0; i < nbs.size(); i++) {
            send_message(nbs[i], msg);
        }
    }

    virtual void compute(MessageContainer& messages)
    {
        if (step_num() == 1) {
            if (id != -1) { //not ctrl
                OWCTYValue_scc& val = value();
                if (val.sccTag == 0) { //not in SCCs found
                    if (val.in_edges.size() == 0) {
                        bcast_to_out_nbs(id); //MSG: <sender>
                        val.out_edges.clear(); //del all out-edges
                        val.color = -1; //mark as trivial
                        val.sccTag = 1;
                    } else if (val.out_edges.size() == 0) {
                        bcast_to_in_nbs(-id - 1); //MSG: <sender>
                        val.in_edges.clear(); //del all out-edges
                        val.color = -1; //mark as trivial
                        val.sccTag = 1;
                    }
                }
            }
        } else {
            OWCTYValue_scc& val = value();
            if (val.sccTag == 0) { //not in SCCs found
                set<VertexID> del_in;
                set<VertexID> del_out;
                for (int i = 0; i < messages.size(); i++) {
                    VertexID message = messages[i];
                    if (message < 0)
                        del_out.insert(-message - 1);
                    else
                        del_in.insert(message);
                }
                vector<VertexID>& in_edges = val.in_edges;
                vector<VertexID> in_new;
                for (int i = 0; i < in_edges.size(); i++) {
                    if (del_in.count(in_edges[i]) == 0)
                        in_new.push_back(in_edges[i]);
                }
                in_edges.swap(in_new);
                vector<VertexID>& out_edges = val.out_edges;
                vector<VertexID> out_new;
                for (int i = 0; i < out_edges.size(); i++) {
                    if (del_out.count(out_edges[i]) == 0)
                        out_new.push_back(out_edges[i]);
                }
                out_edges.swap(out_new);
                if (in_edges.size() == 0) {
                    bcast_to_out_nbs(id); //MSG: <sender>
                    val.out_edges.clear(); //del all out-edges
                    val.color = -1; //mark as trivial
                    val.sccTag = 1;
                } else if (out_edges.size() == 0) {
                    bcast_to_in_nbs(-id - 1); //MSG: <sender>
                    val.in_edges.clear(); //del all out-edges
                    val.color = -1; //mark as trivial
                    val.sccTag = 1;
                }
            }
        }
        vote_to_halt(); //ctrl votes to halt directly
    }
};

class OWCTYWorker_scc : public Worker<OWCTYVertex_scc> {
    char buf[100];

public:
    //C version
    virtual OWCTYVertex_scc* toVertex(char* line)
    {
        char* pch;
        pch = strtok(line, "\t");
        OWCTYVertex_scc* v = new OWCTYVertex_scc;
        v->id = atoi(pch);
        pch = strtok(NULL, " ");
        v->value().color = atoi(pch);
        if (v->id == -1)
            return v;
        pch = strtok(NULL, " ");
        v->value().sccTag = atoi(pch);
        pch = strtok(NULL, " ");
        int num = atoi(pch);
        for (int i = 0; i < num; i++) {
            pch = strtok(NULL, " ");
            v->value().in_edges.push_back(atoi(pch));
        }
        pch = strtok(NULL, " ");
        num = atoi(pch);
        for (int i = 0; i < num; i++) {
            pch = strtok(NULL, " ");
            v->value().out_edges.push_back(atoi(pch));
        }
        return v;
    }

    virtual void toline(OWCTYVertex_scc* v, BufferedWriter& writer)
    {
        if (v->id == -1) {
            sprintf(buf, "-1\t%d\n", v->value().color);
            writer.write(buf);
            return;
        }
        vector<VertexID>& in_edges = v->value().in_edges;
        vector<VertexID>& out_edges = v->value().out_edges;
        sprintf(buf, "%d\t%d %d %d ", v->id, v->value().color, v->value().sccTag, in_edges.size());
        writer.write(buf);
        for (int i = 0; i < in_edges.size(); i++) {
            sprintf(buf, "%d ", in_edges[i]);
            writer.write(buf);
        }
        sprintf(buf, "%d ", out_edges.size());
        writer.write(buf);
        for (int i = 0; i < out_edges.size(); i++) {
            sprintf(buf, "%d ", out_edges[i]);
            writer.write(buf);
        }
        writer.write("\n");
    }
};

void pregel_owcty(string in_path, string out_path)
{
    WorkerParams param;
    param.input_path = in_path;
    param.output_path = out_path;
    param.force_write = true;
    param.native_dispatcher = false;
    OWCTYWorker_scc worker;
    worker.run(param);
}
