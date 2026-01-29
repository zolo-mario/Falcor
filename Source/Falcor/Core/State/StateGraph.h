#pragma once
#include <functional>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Falcor
{
template<typename NodeType, typename EdgeType, typename EdgeHashType = std::hash<EdgeType>>
class StateGraph
{
public:
    using CompareFunc = std::function<bool(const NodeType& data)>;

    StateGraph() : mGraph(1) {}

    bool isEdgeExists(const EdgeType& e) const { return (getEdgeIt(e) != mGraph[mCurrentNode].edges.end()); }

    bool walk(const EdgeType& e)
    {
        if (isEdgeExists(e))
        {
            mCurrentNode = getEdgeIt(e)->second;
            return true;
        }
        else
        {
            // Scan the graph and see if anode w
            uint32_t newIndex = (uint32_t)mGraph.size();
            mGraph[mCurrentNode].edges[e] = newIndex;
            mGraph.push_back(Node());
            mCurrentNode = newIndex;
            return false;
        }
    }

    const NodeType& getCurrentNode() const { return mGraph[mCurrentNode].data; }

    void setCurrentNodeData(const NodeType& data) { mGraph[mCurrentNode].data = data; }

    bool scanForMatchingNode(CompareFunc cmpFunc)
    {
        for (uint32_t i = 0; i < (uint32_t)mGraph.size(); i++)
        {
            if (i != mCurrentNode)
            {
                if (cmpFunc(mGraph[i].data))
                {
                    // Reconnect
                    for (uint32_t n = 0; n < (uint32_t)mGraph.size(); n++)
                    {
                        for (auto& e : mGraph[n].edges)
                        {
                            if (e.second == mCurrentNode)
                            {
                                e.second = i;
                            }
                        }
                    }
                    mCurrentNode = i;
                    return true;
                }
            }
        }
        return false;
    }

private:
    using edge_map = std::unordered_map<EdgeType, uint32_t, EdgeHashType>;

    const auto getEdgeIt(const EdgeType& e) const
    {
        const Node& n = mGraph[mCurrentNode];
        return n.edges.find(e);
    }

    struct Node
    {
        NodeType data = {0};
        edge_map edges;
    };

    std::vector<Node> mGraph;
    uint32_t mCurrentNode = 0;
};
} // namespace Falcor
