import streamlit as st
import networkx as nx
import matplotlib.pyplot as plt
import time
import os

# Try to import the compiled C++ extension
try:
    import neuronet
except ImportError:
    st.error("NeuroNet extension not found. Please compile it using `python setup.py build_ext --inplace`.")
    st.stop()

st.set_page_config(page_title="NeuroNet", layout="wide")

st.title("NeuroNet: Massive Graph Analysis")
st.markdown("### High-Performance Analysis using C++ & Sparse Matrices")

# Sidebar for controls
st.sidebar.header("Control Panel")

# Initialize Engine
if 'engine' not in st.session_state:
    st.session_state.engine = neuronet.NeuroNetEngine()
    st.session_state.data_loaded = False

# File Loading
st.sidebar.subheader("Dataset Ingestion")
uploaded_file = st.sidebar.file_uploader("Upload Edge List (txt)", type=["txt"])
# Option to use local file path for massive files (uploading 100MB+ via browser is slow)
local_path = st.sidebar.text_input("Or specify local file path (e.g., web-Google.txt)")

if st.sidebar.button("Load Dataset"):
    path_to_load = None
    if uploaded_file is not None:
        # Save to temp file
        with open("temp_dataset.txt", "wb") as f:
            f.write(uploaded_file.getbuffer())
        path_to_load = "temp_dataset.txt"
    elif local_path and os.path.exists(local_path):
        path_to_load = local_path
    
    if path_to_load:
        start_time = time.time()
        with st.spinner("Loading Graph into C++ Core..."):
            st.session_state.engine.load_dataset(path_to_load)
        end_time = time.time()
        st.session_state.data_loaded = True
        st.success(f"Graph loaded in {end_time - start_time:.4f} seconds!")
    else:
        st.error("Please provide a valid file.")

# Analysis Section
if st.session_state.data_loaded:
    st.markdown("---")
    col1, col2, col3 = st.columns(3)
    
    num_nodes = st.session_state.engine.get_num_nodes()
    num_edges = st.session_state.engine.get_num_edges()
    
    col1.metric("Nodes", f"{num_nodes:,}")
    col2.metric("Edges", f"{num_edges:,}")
    
    # Max Degree
    if col3.button("Find Max Degree Node"):
        max_node = st.session_state.engine.get_max_degree_node()
        degree = st.session_state.engine.get_degree(max_node)
        col3.metric("Max Degree Node", max_node, delta=f"Degree: {degree}")

    st.markdown("---")
    st.subheader("Graph Traversal & Visualization")
    
    c1, c2 = st.columns(2)
    start_node = c1.number_input("Start Node ID", min_value=0, value=0)
    depth = c2.slider("BFS Depth", 1, 3, 2)
    
    if st.button("Run BFS & Visualize"):
        start_time = time.time()
        visited_nodes = st.session_state.engine.bfs(start_node, depth)
        end_time = time.time()
        
        st.info(f"BFS found {len(visited_nodes)} nodes in {end_time - start_time:.6f} seconds.")
        
        if len(visited_nodes) > 0:
            # Visualize Subgraph using NetworkX
            # We only fetch edges for the visited nodes to keep it light
            G = nx.Graph() # or DiGraph
            
            # Limit visualization size
            max_vis_nodes = 500
            nodes_to_draw = visited_nodes[:max_vis_nodes]
            
            if len(visited_nodes) > max_vis_nodes:
                st.warning(f"Subgraph too large ({len(visited_nodes)} nodes). Showing first {max_vis_nodes}.")
            
            with st.spinner("Constructing Visualization..."):
                for u in nodes_to_draw:
                    neighbors = st.session_state.engine.get_neighbors(u)
                    for v in neighbors:
                        if v in nodes_to_draw: # Only draw internal edges of the subgraph
                            G.add_edge(u, v)
            
            st.write(f"Visualizing {G.number_of_nodes()} nodes and {G.number_of_edges()} edges.")
            
            fig, ax = plt.subplots(figsize=(10, 8))
            pos = nx.spring_layout(G, seed=42)
            
            # Draw
            nx.draw_networkx_nodes(G, pos, node_size=50, node_color='skyblue', alpha=0.8)
            nx.draw_networkx_edges(G, pos, alpha=0.3)
            nx.draw_networkx_labels(G, pos, font_size=8)
            
            # Highlight start node
            if start_node in pos:
                nx.draw_networkx_nodes(G, pos, nodelist=[start_node], node_color='red', node_size=100)
            
            st.pyplot(fig)

else:
    st.info("Please load a dataset to begin analysis.")

