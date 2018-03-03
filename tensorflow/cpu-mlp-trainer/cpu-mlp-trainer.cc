// sample program can be found on  https://tebesu.github.io/posts/Training-a-TensorFlow-graph-in-C++-API

#include "tensorflow/core/public/session.h"
#include "tensorflow/core/graph/default_device.h"

using namespace tensorflow;
using namespace std;

int main(int argc, char* argv[]) {

    string graph_definition = "mlp.pb";
    Session* session;
    GraphDef graph_def;
    SessionOptions opts;
    vector<Tensor> outputs; // Store outputs
    TF_CHECK_OK(ReadBinaryProto(Env::Default(), graph_definition, &graph_def));

    // Set GPU options
    graph::SetDefaultDevice("/cpu:0", &graph_def);
    //opts.config.mutable_gpu_options()->set_per_process_gpu_memory_fraction(0.5);
    //opts.config.mutable_gpu_options()->set_allow_growth(true);

    // create a new session
    TF_CHECK_OK(NewSession(opts, &session));

    // Load graph into session
    TF_CHECK_OK(session->Create(graph_def));

    // Initialize our variables
    TF_CHECK_OK(session->Run({}, {}, {"init_all_vars_op"}, nullptr));

    Tensor x(DT_FLOAT, TensorShape({100, 32}));
    Tensor y(DT_FLOAT, TensorShape({100, 8}));
    auto _XTensor = x.matrix<float>();
    auto _YTensor = y.matrix<float>();

    _XTensor.setRandom();
    _YTensor.setRandom();

    float cost = 0.0;

    TF_CHECK_OK(session->Run({{"x", x}, {"y", y}}, {"cost"}, {}, &outputs)); // Get cost
    float initial_cost = outputs[0].scalar<float>()(0);

    int iter = 0;

    do{
        TF_CHECK_OK(session->Run({{"x", x}, {"y", y}}, {"cost"}, {}, &outputs)); // Get cost
        cost = outputs[0].scalar<float>()(0);
        cout << "Iteration: " << iter << ", Cost: " <<  cost << endl;
        TF_CHECK_OK(session->Run({{"x", x}, {"y", y}}, {}, {"train"}, nullptr)); // Train
        outputs.clear();
        iter++;
    }while( cost >= 0.01 * initial_cost );

    cout << "Final cost: " << cost << endl;

    return 0;
}
