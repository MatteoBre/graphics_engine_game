#include "Node.h"
#include <vector>

Node::Node(mat4 matrix, NodeType nodeType)
{
	children = std::vector<Node*>{};
	this->matrix = matrix;
	this->nodeType = nodeType;
}

void Node::setName(std::string name)
{
	this->name = name;
}

std::string Node::getName()
{
	return name;
}

void Node::addChild(Node* child)
{
	children.push_back(child);
}

int Node::getNumberOfChildren()
{
	return children.size();
}

NodeType Node::getNodeType()
{
	return nodeType;
}

std::vector<Node*>& Node::getChildren()
{
	return children;
}

mat4 Node::getMatrix()
{
	return matrix;
}

void Node::setMatrix(mat4 matrix)
{
	this->matrix = matrix;
}

void Node::draw(mat4 matrix)
{
}
