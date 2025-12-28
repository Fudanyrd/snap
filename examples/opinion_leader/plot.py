import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
try:
    import networkx as nx
    HAS_NETWORKX = True
except ImportError:
    HAS_NETWORKX = False
    print("[WARNING] networkx not installed. Network structure visualization will be skipped.")

RESULTS = Path("results")
OUT = Path("viz")
OUT.mkdir(exist_ok=True)

DATASETS = ["facebook", "citation", "wiki-vote"]

# 根据实际输出文件的列顺序
CENTRALITY_COLUMNS = [
    "NodeId",
    "Degree",
    "PageRank",
    "HubScore",
    "AuthorityScore",
    "Betweenness",
    "Closeness",
    "Eigenvector",
    "ClusteringCoeff",
    "CompositeScore",
    "Rank"
]

def load_centrality(dataset):
    """加载中心性数据文件"""
    path = RESULTS / f"{dataset}_centrality.tab"
    if not path.exists():
        print(f"[ERROR] File not found: {path}")
        return None
    
    try:
        df = pd.read_csv(
            path,
            sep="\t",
            comment="#",
            skiprows=3,  # 跳过注释行和标题行
            names=CENTRALITY_COLUMNS
        )
        return df
    except Exception as e:
        print(f"[ERROR] Failed to load {path}: {e}")
        return None

def plot_centrality_distribution(dataset, col, use_log=False):
    """绘制中心性指标分布图"""
    df = load_centrality(dataset)
    if df is None or col not in df.columns:
        print(f"[SKIP] {dataset}: column '{col}' not found")
        return

    # 过滤掉零值（对于某些指标可能有很多零值）
    data = df[col].replace(0, np.nan).dropna()
    if len(data) == 0:
        print(f"[SKIP] {dataset}: {col} has no non-zero values")
        return

    plt.figure(figsize=(8, 6))
    if use_log:
        # 使用对数刻度
        plt.hist(data, bins=50, edgecolor='black', alpha=0.7)
        plt.yscale('log')
        plt.xscale('log')
    else:
        plt.hist(data, bins=50, edgecolor='black', alpha=0.7)
    
    plt.xlabel(col, fontsize=12)
    plt.ylabel("Number of nodes", fontsize=12)
    plt.title(f"{dataset.capitalize()}: {col} Distribution", fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    filename = f"{dataset}_{col}_dist.pdf"
    plt.savefig(OUT / filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"[OK] Generated: {filename}")

def plot_top50(dataset):
    """绘制Top-50意见领袖条形图"""
    path = RESULTS / f"{dataset}_top50_leaders.txt"
    if not path.exists():
        print(f"[ERROR] File not found: {path}")
        return
    
    try:
        df = pd.read_csv(path, sep="\t", comment="#")
        
        if "CompositeScore" not in df.columns:
            print(f"[SKIP] {dataset}: CompositeScore column not found")
            return

        plt.figure(figsize=(12, 6))
        ranks = df["Rank"]
        scores = df["CompositeScore"]
        
        plt.bar(ranks, scores, color='steelblue', edgecolor='black', alpha=0.7)
        plt.xlabel("Rank", fontsize=12)
        plt.ylabel("Composite Score", fontsize=12)
        plt.title(f"{dataset.capitalize()}: Top-50 Opinion Leaders", fontsize=14, fontweight='bold')
        plt.grid(True, alpha=0.3, axis='y')
        plt.tight_layout()
        
        filename = f"{dataset}_top50.pdf"
        plt.savefig(OUT / filename, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"[OK] Generated: {filename}")
    except Exception as e:
        print(f"[ERROR] Failed to plot top50 for {dataset}: {e}")

def plot_centrality_comparison(dataset):
    """绘制多个中心性指标的对比图"""
    df = load_centrality(dataset)
    if df is None:
        return
    
    # 选择Top-20节点进行对比
    top20 = df.nlargest(20, "CompositeScore")
    
    metrics = ["PageRank", "Betweenness", "Eigenvector", "ClusteringCoeff"]
    available_metrics = [m for m in metrics if m in df.columns]
    
    if len(available_metrics) < 2:
        print(f"[SKIP] {dataset}: Not enough metrics for comparison")
        return
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    axes = axes.flatten()
    
    for idx, metric in enumerate(available_metrics[:4]):
        ax = axes[idx]
        data = top20[metric].replace(0, np.nan).dropna()
        if len(data) > 0:
            ax.barh(range(len(data)), data.values, color='coral', edgecolor='black', alpha=0.7)
            ax.set_yticks(range(len(data)))
            ax.set_yticklabels([f"Rank {r}" for r in top20.iloc[:len(data)]["Rank"]])
            ax.set_xlabel(metric, fontsize=10)
            ax.set_title(f"Top-20 by {metric}", fontsize=11, fontweight='bold')
            ax.grid(True, alpha=0.3, axis='x')
    
    plt.suptitle(f"{dataset.capitalize()}: Centrality Metrics Comparison (Top-20)", 
                 fontsize=14, fontweight='bold', y=0.995)
    plt.tight_layout()
    
    filename = f"{dataset}_centrality_comparison.pdf"
    plt.savefig(OUT / filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"[OK] Generated: {filename}")

def load_graph(dataset, max_nodes=1000):
    """加载图数据，对于大图进行采样"""
    data_path = Path("data")
    graph_files = {
        "facebook": "facebook_combined.txt",
        "citation": "cit-HepPh.txt",
        "wiki-vote": "wiki-Vote.txt"
    }
    
    if dataset not in graph_files:
        print(f"[ERROR] Unknown dataset: {dataset}")
        return None, None
    
    graph_file = data_path / graph_files[dataset]
    if not graph_file.exists():
        print(f"[ERROR] Graph file not found: {graph_file}")
        return None, None
    
    if not HAS_NETWORKX:
        return None, None
    
    try:
        # 读取边列表
        edges = []
        with open(graph_file, 'r') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                parts = line.split()
                if len(parts) >= 2:
                    try:
                        src, dst = int(parts[0]), int(parts[1])
                        edges.append((src, dst))
                    except ValueError:
                        continue
        
        # 创建图
        G = nx.Graph() if dataset == "facebook" else nx.DiGraph()
        G.add_edges_from(edges)
        
        print(f"[INFO] Loaded graph: {G.number_of_nodes()} nodes, {G.number_of_edges()} edges")
        
        # 如果节点太多，进行采样
        if G.number_of_nodes() > max_nodes:
            print(f"[INFO] Graph too large, sampling top {max_nodes} nodes by degree...")
            # 按度排序，选择度最大的节点及其邻居
            degrees = dict(G.degree())
            top_nodes = sorted(degrees.items(), key=lambda x: x[1], reverse=True)[:max_nodes//2]
            top_node_ids = [n[0] for n in top_nodes]
            
            # 包含这些节点的邻居
            sampled_nodes = set(top_node_ids)
            for node in top_node_ids:
                sampled_nodes.update(list(G.neighbors(node))[:5])  # 每个节点最多5个邻居
            
            if len(sampled_nodes) > max_nodes:
                sampled_nodes = list(sampled_nodes)[:max_nodes]
            
            G = G.subgraph(sampled_nodes).copy()
            print(f"[INFO] Sampled graph: {G.number_of_nodes()} nodes, {G.number_of_edges()} edges")
        
        return G, graph_files[dataset]
    except Exception as e:
        print(f"[ERROR] Failed to load graph: {e}")
        return None, None

def plot_network_structure(dataset, centrality_metric="CompositeScore", max_nodes=1000):
    """绘制网络结构图，节点大小和颜色根据中心性度量"""
    if not HAS_NETWORKX:
        print(f"[SKIP] {dataset}: networkx not available")
        return
    
    # 加载图和中心性数据
    G, graph_file = load_graph(dataset, max_nodes)
    if G is None:
        return
    
    df = load_centrality(dataset)
    if df is None:
        return
    
    # 创建节点ID到中心性值的映射
    if centrality_metric not in df.columns:
        print(f"[SKIP] {dataset}: centrality metric '{centrality_metric}' not found")
        return
    
    centrality_dict = dict(zip(df['NodeId'], df[centrality_metric]))
    
    # 只保留图中存在的节点
    node_centrality = {n: centrality_dict.get(n, 0) for n in G.nodes()}
    
    # 过滤掉零值
    non_zero_centrality = {k: v for k, v in node_centrality.items() if v > 0}
    if len(non_zero_centrality) == 0:
        print(f"[SKIP] {dataset}: No nodes with non-zero {centrality_metric}")
        return
    
    # 使用增强的对数归一化来最大化颜色差异
    values = np.array(list(non_zero_centrality.values()))
    min_val = np.min(values)
    max_val = np.max(values)
    
    if max_val > min_val:
        # 方法1: 对数归一化（适合幂律分布）
        # 先进行对数变换来压缩高值，拉伸低值
        log_values = np.log1p(values - min_val)
        log_max = np.max(log_values)
        
        if log_max > 0:
            # 对数归一化到[0, 1]
            log_normalized = log_values / log_max
        else:
            log_normalized = np.ones_like(log_values) * 0.5
        
        # 方法2: 使用分位数归一化来增强极端值的差异
        # 计算分位数，使高值区域更突出
        q75 = np.percentile(values, 75)
        q25 = np.percentile(values, 25)
        median = np.median(values)
        
        # 对高值区域使用更激进的映射
        enhanced_normalized = np.zeros_like(log_normalized)
        for i, val in enumerate(values):
            if val >= q75:
                # 高值区域：映射到[0.7, 1.0]，使用平方根拉伸
                ratio = (val - q75) / (max_val - q75 + 1e-10)
                enhanced_normalized[i] = 0.7 + 0.3 * np.sqrt(ratio)
            elif val >= median:
                # 中高值区域：映射到[0.4, 0.7]
                ratio = (val - median) / (q75 - median + 1e-10)
                enhanced_normalized[i] = 0.4 + 0.3 * ratio
            elif val >= q25:
                # 中低值区域：映射到[0.15, 0.4]
                ratio = (val - q25) / (median - q25 + 1e-10)
                enhanced_normalized[i] = 0.15 + 0.25 * ratio
            else:
                # 低值区域：映射到[0.0, 0.15]，压缩低值
                ratio = (val - min_val) / (q25 - min_val + 1e-10)
                enhanced_normalized[i] = 0.0 + 0.15 * np.sqrt(ratio)
        
        # 对于零值节点，使用最小值
        centrality_normalized = dict(zip(non_zero_centrality.keys(), enhanced_normalized))
    else:
        centrality_normalized = {k: 0.5 for k in non_zero_centrality.keys()}
    
    # 设置节点大小（根据中心性，使用更大的范围增强差异）
    # 使用更大的倍数范围：从50到800
    node_sizes = []
    for n in G.nodes():
        norm_val = centrality_normalized.get(n, 0.05)
        # 使用平方来增强大小差异
        size = 50 + (norm_val ** 1.5) * 750
        node_sizes.append(size)
    
    # 设置节点颜色（使用更明显的颜色映射）
    # 直接使用归一化值，分位数归一化已经增强了差异
    node_colors = []
    for n in G.nodes():
        norm_val = centrality_normalized.get(n, 0.0)
        # 对于零值节点，使用很小的值以便在colormap中显示为深色
        node_colors.append(max(norm_val, 0.01) if norm_val > 0 else 0.01)
    
    # 选择布局算法
    print(f"[INFO] Computing layout for {G.number_of_nodes()} nodes...")
    if G.number_of_nodes() < 300:
        pos = nx.spring_layout(G, k=1.5, iterations=100, seed=42)
    elif G.number_of_nodes() < 500:
        pos = nx.spring_layout(G, k=1, iterations=50, seed=42)
    else:
        # 对于大图，使用更快的布局算法
        try:
            pos = nx.kamada_kawai_layout(G)
        except:
            pos = nx.spring_layout(G, k=0.5, iterations=30, seed=42)
    
    # 绘制网络图
    fig, ax = plt.subplots(figsize=(16, 12))
    
    # 绘制边（对于有向图，使用箭头）
    if isinstance(G, nx.DiGraph):
        nx.draw_networkx_edges(G, pos, alpha=0.15, width=0.3, 
                              edge_color='gray', arrows=True, 
                              arrowsize=10, arrowstyle='->', ax=ax)
    else:
        nx.draw_networkx_edges(G, pos, alpha=0.15, width=0.3, 
                              edge_color='gray', ax=ax)
    
    # 绘制节点（使用更明显的颜色映射）
    # 使用inferno colormap：从黑色到黄色，对比度极高，非常适合显示中心性差异
    # 或者使用plasma：从深紫色到亮黄色
    # 或者使用coolwarm：从蓝色到红色，冷暖对比明显
    nodes = nx.draw_networkx_nodes(
        G, pos,
        node_size=node_sizes,
        node_color=node_colors,
        cmap=plt.cm.inferno,  # inferno: 黑色->紫色->红色->黄色，对比度极高
        alpha=0.95,  # 提高不透明度使颜色更鲜艳
        vmin=0,
        vmax=1,
        edgecolors='white',  # 白色边框使节点更清晰，与深色背景形成对比
        linewidths=1.0,  # 稍微加粗边框
        ax=ax
    )
    
    # 标注Top节点（只标注前5个，避免过于拥挤）
    top_nodes = sorted(node_centrality.items(), key=lambda x: x[1], reverse=True)[:5]
    top_node_ids = [n[0] for n in top_nodes if n[0] in G.nodes()]
    
    # 为Top节点添加标签和特殊标记
    if top_node_ids:
        # 创建节点到索引的映射
        node_list = list(G.nodes())
        node_to_idx = {n: idx for idx, n in enumerate(node_list)}
        
        # 高亮Top节点
        top_node_sizes = [node_sizes[node_to_idx[n]] * 1.3 for n in top_node_ids]
        nx.draw_networkx_nodes(
            G, pos,
            nodelist=top_node_ids,
            node_size=top_node_sizes,
            node_color='red',
            alpha=0.9,
            ax=ax
        )
        
        # 添加标签（显示排名和节点ID）
        sorted_centrality = sorted(node_centrality.items(), key=lambda x: x[1], reverse=True)
        labels = {}
        for n in top_node_ids:
            rank = next(i+1 for i, (node, _) in enumerate(sorted_centrality) if node == n)
            labels[n] = f"#{rank}\n{n}"
        
        nx.draw_networkx_labels(G, pos, labels, font_size=9, 
                               font_weight='bold', font_color='white',
                               bbox=dict(boxstyle='round,pad=0.3', facecolor='red', alpha=0.7),
                               ax=ax)
    
    # 添加颜色条
    if nodes is not None:
        cbar = plt.colorbar(nodes, ax=ax, label=centrality_metric, shrink=0.6, pad=0.02)
        cbar.ax.tick_params(labelsize=10)
    
    # 设置标题
    graph_type = "Undirected" if isinstance(G, nx.Graph) and not isinstance(G, nx.DiGraph) else "Directed"
    ax.set_title(f"{dataset.capitalize()}: Network Structure Visualization\n"
                 f"Node size and color based on {centrality_metric} | {graph_type} Graph\n"
                 f"({G.number_of_nodes()} nodes, {G.number_of_edges()} edges)",
                 fontsize=14, fontweight='bold', pad=20)
    ax.axis('off')
    plt.tight_layout()
    
    filename = f"{dataset}_network_structure_{centrality_metric.lower()}.pdf"
    plt.savefig(OUT / filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"[OK] Generated: {filename}")

def plot_network_structure_multiple_metrics(dataset, max_nodes=1000):
    """为多个中心性度量生成网络结构图"""
    metrics = ["CompositeScore", "PageRank", "Betweenness", "Degree"]
    
    for metric in metrics:
        plot_network_structure(dataset, centrality_metric=metric, max_nodes=max_nodes)

if __name__ == "__main__":
    print("=" * 60)
    print("Generating visualizations for opinion leader results")
    print("=" * 60)
    
    # 验证：打印一次列名
    test_df = load_centrality("facebook")
    if test_df is not None:
        print(f"[OK] Centrality columns: {test_df.columns.tolist()}")
    print()

    for ds in DATASETS:
        print(f"Processing dataset: {ds}")
        print("-" * 60)
        
        # 分布图
        plot_centrality_distribution(ds, "Degree", use_log=True)
        plot_centrality_distribution(ds, "PageRank", use_log=True)
        plot_centrality_distribution(ds, "Betweenness", use_log=True)
        plot_centrality_distribution(ds, "Eigenvector", use_log=False)
        
        # Top-50图
        plot_top50(ds)
        
        # 对比图
        plot_centrality_comparison(ds)
        
        # 网络结构图（根据数据集大小调整采样）
        if ds == "facebook":
            plot_network_structure_multiple_metrics(ds, max_nodes=500)
        elif ds == "wiki-vote":
            plot_network_structure_multiple_metrics(ds, max_nodes=800)
        else:  # citation - 最大的图
            plot_network_structure_multiple_metrics(ds, max_nodes=1000)
        
        print()
    
    print("=" * 60)
    print(f"All figures generated in: {OUT.absolute()}")
    print("=" * 60)
