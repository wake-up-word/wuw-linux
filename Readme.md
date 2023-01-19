# About

https://www.intechopen.com/chapters/15946

# WUW Linux
Prerequisites

cmake >= 3.8
gcc >= 10.2.0

```
sudo apt install build-essential cmake git
```

Build with cmake, use: https://code.visualstudio.com/docs/cpp/cmake-linux


----
```
mkdir build
cd build
cmake ../
cmake --build .
```
# Build with docker

Build and compile in docker, copy build results to `./build`
```
docker build . -t wuw && \
CID=$(docker create wuw) && \
docker cp ${CID}:/app/build ./ && \
docker rm ${CID}
```

# Manually running
```
./build/run_wuw \
    -i ./config/audio/WUWII00000_001.ulaw \
    -d ./out \
    -o ./fe \
    -S ./config/operator3.s 0.9 0 \
    -M ./config/operator3.m \
    ./config/operator3.l \
    ./config/operator3.e
```

```
./build/run_wuw \
    -i ./config/audio/WUWII00000_001.ulaw \
    -d ./out \
    -o ./fe \
    -S ./test/new_model.svm  0.2 0 \
    -M ./test/trained_model_1.bhmm \
    ./test/trained_model_2.bhmm \
    ./test/trained_model_3.bhmm
```

# Generate binary hmm model
```
./run_gen_hmm 39 30 6 0 1 1 ./test.bhmm
```
> Gen Model
> 1. Number of phones in wuw (operator 8 /ˈäpəˌrādər/)
> 2. At least 3 hmm states per phone, 8*3 n_states
> 3. 3-5 mixtures optimal (from experimental testing) Slide 200 CH5 slides for speech recognition
> 4. none, we want whole word
> 5. sil beginning 1 state, ending 1 state
> 6. num states 13*3 = 39 (3: one for static, first dirivitive, second derivitive, 13: from code NUM_DCT_ELEMENTS)

# Train hmm models
Each feature is trained independently for now.

> TODO train all 3 features at same time (memory error bug)
```
./train_hmm
-I ./train_input.txt \
-M ./test.bhmm \
-s true \
-t 10 \
-x 3 1 \

./train_hmm
-I ./train_input.txt \
-M ./test.bhmm \
-s true \
-t 10 \
-x 3 2 \

./train_hmm
-I ./train_input.txt \
-M ./test.bhmm \
-s true \
-t 10 \
-x 3 3 \
```

# Generate hmm scores for svm model
Each score is generated independently for now.

> TODO score all 3 features at same time (memory error bug)
```
./train_hmm
-I ./train_input.txt \
-M ./trained_model_1.bhmm \
-x 3 1 \

./train_hmm
-I ./train_input.txt \
-M ./trained_model_2.bhmm \
-x 3 2 \

./train_hmm
-I ./train_input.txt \
-M ./trained_model_3.bhmm \
-x 3 3 \
```


# Build Example Model "Operator"
Using Docker
```
docker build -f ./Dockerfile.operator_model -t wuw_operator .
CID=$(docker create wuw_operator) && \
docker cp ${CID}:/app/out ./ && \
docker rm ${CID}
```