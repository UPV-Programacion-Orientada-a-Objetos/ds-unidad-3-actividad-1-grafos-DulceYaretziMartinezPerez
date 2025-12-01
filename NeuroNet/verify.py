import neuronet
import os

def test_neuronet():
    print("Initializing NeuroNet Engine...")
    engine = neuronet.NeuroNetEngine()
    
    # Create a dummy file
    with open("verify_data.txt", "w") as f:
        f.write("0 1\n0 2\n1 3\n2 4\n")
        
    print("Loading dummy dataset...")
    engine.load_dataset("verify_data.txt")
    
    print(f"Nodes: {engine.get_num_nodes()}")
    print(f"Edges: {engine.get_num_edges()}")
    
    print("Running BFS from node 0...")
    visited = engine.bfs(0, 2)
    print(f"Visited nodes: {visited}")
    
    if len(visited) > 0:
        print("SUCCESS: NeuroNet is working correctly.")
    else:
        print("FAILURE: BFS returned empty list.")

    # Clean up
    if os.path.exists("verify_data.txt"):
        os.remove("verify_data.txt")

if __name__ == "__main__":
    test_neuronet()
