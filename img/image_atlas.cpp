#include "image_atlas.h"

#include "common/clock.h"
#include "common/log.h"
#include "painter.h"

namespace hc
{

namespace
{

struct Node
{
    typedef std::shared_ptr<Node> Ptr;

    Rect<int> rect;
    Node::Ptr nodes[2];
    bool reserved {};

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
            if (reserved)
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

    void draw(Painter* painter, uint32_t color) const
    {
        painter->setColor(color);
        painter->drawRect(rect);
        if (nodes[0]) nodes[0]->draw(painter, 0xff0000ff);
        if (nodes[1]) nodes[1]->draw(painter, 0xffff0000);
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

Size<int> ImageAtlas::size() const
{
    return d->atlas.size();
}

Image ImageAtlas::image(bool drawNodes) const
{
    Image atlas = d->atlas;
    if (drawNodes)
    {
        atlas = atlas.clone();
        Painter painter(&atlas);
        d->root.draw(&painter, 0xff0000ff);
    }
    return atlas;
}

Rect<int> ImageAtlas::insert(const Image& image, int margin)
{
    if (Node* node = d->root.reserve(image.size() + 2 * margin))
    {
        // Blit the image into atlas
        Painter painter(&d->atlas);
        painter.drawImageClamped(image, node->rect.x, node->rect.y, margin);

        // Mark node reserved
        node->reserved = true;
        return node->rect;
    }
    return Rect<int>();
}

RectCube<int> ImageAtlas::insert(const ImageCube& imageCube, int margin)
{
    HCTIME("cube");

    RectCube<int> cubeRect;
    for (std::size_t i = 0; i < imageCube.sides.size(); ++i)
        cubeRect[i] = insert(imageCube.side(ImageCube::Side(i)), margin);

    return cubeRect;
}

} // namespace hc
