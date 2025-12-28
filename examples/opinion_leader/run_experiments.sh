#!/bin/bash
# Script to run opinion leader mining on all three datasets

echo "=========================================="
echo "Opinion Leader Mining Experiments"
echo "=========================================="

# Check if program is compiled
if [ ! -f "opinion_leader" ]; then
    echo "Compiling opinion_leader program..."
    make all
    if [ $? -ne 0 ]; then
        echo "Compilation failed!"
        exit 1
    fi
fi

# Create results directory
mkdir -p results

# Dataset 1: ego-Facebook (undirected social network)
echo ""
echo "=========================================="
echo "Experiment 1: ego-Facebook Network"
echo "=========================================="
if [ -f "data/facebook_combined.txt" ]; then
    ./opinion_leader -i:data/facebook_combined.txt \
                     -o:results/facebook \
                     -d:false \
                     -k:50 \
                     -t:"Facebook Ego Network" \
                     -p:true
    echo "Facebook experiment completed!"
else
    echo "Warning: facebook_combined.txt not found. Skipping..."
fi

# Dataset 2: Wiki-Vote (directed voting network)
echo ""
echo "=========================================="
echo "Experiment 2: Wiki-Vote Network"
echo "=========================================="
if [ -f "data/wiki-Vote.txt" ]; then
    ./opinion_leader -i:data/wiki-Vote.txt \
                     -o:results/wiki-vote \
                     -d:true \
                     -k:50 \
                     -t:"Wikipedia Vote Network" \
                     -p:true
    echo "Wiki-Vote experiment completed!"
else
    echo "Warning: wiki-Vote.txt not found. Skipping..."
fi

# Dataset 3: Citation Network (directed citation network)
echo ""
echo "=========================================="
echo "Experiment 3: Citation Network"
echo "=========================================="
if [ -f "data/citation_network.txt" ]; then
    ./opinion_leader -i:data/citation_network.txt \
                     -o:results/citation \
                     -d:true \
                     -k:50 \
                     -t:"Citation Network" \
                     -p:true
    echo "Citation network experiment completed!"
elif [ -f "data/cit-HepPh.txt" ]; then
    # Use cit-HepPh as alternative
    ./opinion_leader -i:data/cit-HepPh.txt \
                     -o:results/citation \
                     -d:true \
                     -k:50 \
                     -t:"Citation Network (HepPh)" \
                     -p:true
    echo "Citation network (HepPh) experiment completed!"
else
    echo "Warning: citation_network.txt not found. Skipping..."
fi

echo ""
echo "=========================================="
echo "All experiments completed!"
echo "Results are saved in the results/ directory"
echo "=========================================="

