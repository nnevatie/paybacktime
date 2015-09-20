#include "image_atlas.h"

namespace hc
{

namespace
{

struct Node
{
    typedef std::shared_ptr<Node> Ptr;

    int id {};
    Rect<int> rect;
    Node::Ptr nodes[2];

    Node()
    {}

    Node(const Rect<int>& rect) : rect(rect)
    {}

    bool isLeaf() const
    {
        return !nodes[0] && !nodes[1];
    }

    Node* reserve(const Size<int>& size)
    {
        if (isLeaf())
        {
            // Already contains data?
            if (id)
                return nullptr;

            // Too small to fit?
            if (rect.size.w < size.w || rect.size.h < size.h)
                return nullptr;

            // Exact fit?
            if (rect.size.w == size.w && rect.size.h == size.h)
                return this;

            // Create subnodes
            nodes[0] = Node::Ptr(new Node());
            nodes[1] = Node::Ptr(new Node());

            // Size deltas
            const int dw = rect.size.w - size.w;
            const int dh = rect.size.h - size.h;

            // Determine split axis
            if (dw > dh)
            {
                // Horizontal
                nodes[0]->rect = Rect<int>(rect.x, rect.y,
                                           size.w, rect.size.h);
                nodes[1]->rect = Rect<int>(rect.x + size.w, rect.y,
                                           rect.size.w - size.w, rect.size.h);
            }
            else
            {
                // Vertical
                nodes[0]->rect = Rect<int>(rect.x, rect.y,
                                           rect.size.w, size.h);
                nodes[1]->rect = Rect<int>(rect.x, rect.y + size.h,
                                           rect.size.w, rect.size.h - size.h);
            }
            // Reserve from first subnode
            return nodes[0]->reserve(size);
        }
        else
        {
            // Try first subnode, then second
            Node* node = nodes[0]->reserve(size);
            return node ? node : nodes[1]->reserve(size);
        }
    }
};

}

struct ImageAtlas::Data
{
    Image atlas;
    Node  root;

    Data(const Size<int>& size) :
        atlas(size, 4), root(size)
    {
        atlas.fill(0xff000000);
    }
};

ImageAtlas::ImageAtlas(const Size<int>& size) :
    d(new Data(size))
{
}

Image ImageAtlas::atlas() const
{
    return d->atlas;
}

bool ImageAtlas::insert(const Image& image)
{
    Node* node = d->root.reserve(image.size());
    if (node)
    {
        return true;
    }
    return false;
}

void ImageAtlas::insert(const ImageCube& imageCube)
{
    for (const Image& image : imageCube.sides)
        insert(image);
}

} // namespace hc
