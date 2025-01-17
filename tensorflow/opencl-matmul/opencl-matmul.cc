#include <sys/time.h>
#include <time.h>
#include <random>

#include "tensorflow/core/public/session.h"
#include "tensorflow/core/graph/default_device.h"

using namespace tensorflow;
using namespace std;

int main(int argc, char* argv[]) {

    if( argc != 8 ){
      cerr << "expected 2 arguments [rowA] [colA] [rowB] [colB] [TransA] [TransB] [Num of Runs]" << endl;
      exit(1);
    }

    // Random generator
    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::normal_distribution<> dis{0,5};

    // Timers
    struct timeval start, end;

    string graph_definition = "matmul.pb";
    Session* session;
    GraphDef graph_def;
    SessionOptions opts;
    vector<Tensor> outputs; // Store outputs
    TF_CHECK_OK(ReadBinaryProto(Env::Default(), graph_definition, &graph_def));

    // Set graph options
    graph::SetDefaultDevice("/cpu:0", &graph_def);

    // create a new session
    TF_CHECK_OK(NewSession(opts, &session));

    // Load graph into session
    TF_CHECK_OK(session->Create(graph_def));

    // Matrix transpose option before matrix multiplication
    int transA = atoi( argv[5] );
    int transB = atoi( argv[6] );

    // Matrix size
    int rowA = atoi( argv[1] );
    int colA = atoi( argv[2] );
    int rowB = atoi( argv[3] );
    int colB = atoi( argv[4] );
    int rowC = (transA == 1) ? colA : rowA;
    int colC = (transB == 1) ? rowB : colB;

    // Number of runs
    int num_runs = atoi( argv[7] );

    // Tensorflow Tensor initializaiotn
    Tensor TensorA (DT_FLOAT, TensorShape({ rowA, colA }));
    Tensor TensorB (DT_FLOAT, TensorShape({ rowB, colB }));
    float * TensorC = (float*)malloc( rowC * colC * sizeof(float) );

    // Matrix initializaiotn
    auto TensorAMatrix = TensorA.tensor<float, 2>();
    for( int i = 0 ; i < rowA ; i ++ ){
      for( auto j = 0 ; j < colA ; j ++ ){
        TensorAMatrix(i, j) = dis(gen);
      }
    }
    auto TensorBMatrix = TensorB.tensor<float, 2>();
    for( int i = 0 ; i < rowB ; i ++ ){
      for( auto j = 0 ; j < colB ; j ++ ){
        TensorBMatrix(i, j) = dis(gen);
      }
    }

    LOG(INFO) << ">>> [TF] Starting " << num_runs << " TF MatMul runs...";
    // Start timer
    gettimeofday(&start, NULL);

    for (int r=0; r<num_runs; r++) {
      // Compute matrix multiplaction result using TF
      TF_CHECK_OK(session->Run({{"x", TensorA}, {"y", TensorB}}, {"matmul"},
        {}, &outputs)); // Get cost
    }
    auto tf_res = outputs[0].matrix<float>();
    // cout << "TF result: \n" << tf_res << endl;

    // Stop timer
    gettimeofday(&end, NULL);

    double interval = ( end.tv_sec * 1.0e6 + end.tv_usec ) -
      ( start.tv_sec * 1.0e6 + start.tv_usec );
    double runtime = interval / num_runs;
    LOG(INFO) << ">>> Done: took " << runtime << " us per run";

    // Compute matrix multiplaction result using Eigen
    auto TensorAEigenMap = Eigen::Map<Eigen::Matrix<
      float,           /* scalar element type */
      Eigen::Dynamic,  /* num_rows is a run-time value */
      Eigen::Dynamic,  /* num_cols is a run-time value */
      Eigen::RowMajor  /* tensorflow::Tensor is always row-major */>>(
        TensorA.flat<float>().data(),  /* ptr to data */
        rowA,           /* num_rows */
        colA            /* num_cols */);

    auto TensorBEigenMap = Eigen::Map<Eigen::Matrix<
      float,           /* scalar element type */
      Eigen::Dynamic,  /* num_rows is a run-time value */
      Eigen::Dynamic,  /* num_cols is a run-time value */
      Eigen::RowMajor  /* tensorflow::Tensor is always row-major */>>(
        TensorB.flat<float>().data(),  /* ptr to data */
        rowB,           /* num_rows */
        colB            /* num_cols */);

    auto eigen_res = Eigen::Map<Eigen::Matrix<
      float,           /* scalar element type */
      Eigen::Dynamic,  /* num_rows is a run-time value */
      Eigen::Dynamic,  /* num_cols is a run-time value */
      Eigen::RowMajor  /* tensorflow::Tensor is always row-major */>>(
        TensorC,  /* ptr to data */
        rowC,           /* num_rows */
        colC            /* num_cols */);

    LOG(INFO) << ">>> [Eigen] Starting " << num_runs << " Eigen MatMul runs...";
    // Start timer
    gettimeofday(&start, NULL);

    if( transA == 1 && transB == 1 ){
      eigen_res = ( TensorAEigenMap.transpose() * TensorBEigenMap.transpose() );
    }else if( transA == 1 && transB == 0 ){
      eigen_res = ( TensorAEigenMap.transpose() * TensorBEigenMap );
    }else if( transA == 0 && transB == 1 ){
      eigen_res = ( TensorAEigenMap * TensorBEigenMap.transpose() );
    }else if( transA == 0 && transB == 0 ){
      eigen_res = ( TensorAEigenMap * TensorBEigenMap );
    }

    // cout << "Eigen result: \n" << eigen_res << endl ;

    // Stop timer
    gettimeofday(&end, NULL);
    interval = ( end.tv_sec * 1.0e6 + end.tv_usec ) -
      ( start.tv_sec * 1.0e6 + start.tv_usec );
    runtime = interval;
    LOG(INFO) << ">>> Done: took " << runtime << " us per run";

    cout << "Checking results ...\n";

    double accu_err = 0;
    double signErrCount = 0;
    double valueErrCount = 0;
    for( auto row = 0 ; row < rowC ; row ++ )
    {
      for( auto col = 0 ; col < colC ; col ++ ){
        float tmp = abs( tf_res(row, col) - eigen_res(row, col) );
        accu_err += tmp;
        if(  tf_res(row, col) * eigen_res(row, col) < 0 ){
          // cout << "(" << row << "," << col << ") sign err, tf_res " << tf_res(row, col) << " eigen_res " << eigen_res(row, col) << endl;
          signErrCount++;
        }
        else if( tmp > 1 ){
          // cout << "(" << row << "," << col << ") val err, tf_res " << tf_res(row, col) << " eigen_res " << eigen_res(row, col) << endl;
          valueErrCount++;
        }
      }
    }
    cout << "err per unit: " << accu_err/(rowC*colC) << ", signErr(%) " << signErrCount/(rowC*colC) << ", valueErr(%) " << valueErrCount/(rowC*colC) << endl;

    free(TensorC);
    
    return 0;
}
