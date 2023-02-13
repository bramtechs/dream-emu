// descriptors

static Description DescribeComponentSprite(void* data){
    auto sprite = (Sprite*) data;
    Vector2 center = sprite->center();
    BoundingBox2D b = sprite->bounds;
    bool isVisible = sprite->isVisible;
    return { STRING(Sprite), TextFormat("Center: %f %f\nBounds: %f %f\n %f %f\nVisible: %d",center.x,center.y,b.min.x,b.min.y,b.max.x,b.max.y,isVisible), SKYBLUE };
}

static Description DescribeComponentPhysicsBody(void* data){
    auto phys = (PhysicsBody*) data;
    if (phys->body){
        b2Vec2 pos = phys->body->GetPosition();
        b2Vec2 vel = phys->body->GetLinearVelocity();
        float mass = phys->body->GetMass();
        float inertia = phys->body->GetInertia();

        return { STRING(PhysicsBody), TextFormat("Phys Pos: %f %f\nVel: %f %f\nDynamic: %d\nMass: %f kg\nInertia: %f",
                    pos.x,pos.y,vel.x,vel.y,phys->dynamic,mass,inertia), PURPLE
        };
    }
    return { STRING(PhysicsBody), "Not initialized!" };
}

static Description DescribeComponentAnimationPlayer(void* data){
    auto anim = (AnimationPlayer*) data;
    return { STRING(AnimationPlayer), TextFormat("Frame: %d\nAnim: %s\nFPS: %f",
            abs(anim->curFrame),anim->curAnim->name.c_str(),anim->curAnim->fps), YELLOW 
    };
}

static Description DescribeComponentPlatformerPlayer(void* data){
    auto player = (PlatformerPlayer*) data;
    const char* poseName = PlayerPoseNames[player->pose];
    return { STRING(PlatformerPlayer), TextFormat("Pose: %s",poseName), GREEN };
}

#ifdef MAGMA_3D
static Description DescribeComponentBase(void* data){
    auto base = (Base*) data;
    BoundingBox b = base->bounds;
    return { STRING(Base), TextFormat("Bounds: %f %f %f\n %f %f %f",b.min.x,b.min.y,b.min.z,b.max.x,b.max.y,b.max.z), RED };
}

static Description DescribeComponentModelRenderer(void* data){
    auto renderer = (ModelRenderer*) data;
    return { STRING(ModelRenderer), TextFormat("Model: %s\nAccurate: %d\nOffset: %f %f %f",
                renderer->model,renderer->accurate,
                renderer->offset.x,renderer->offset.y,renderer->offset.z), PINK
    };
}
#endif

// spawners
static EntityID SpawnWallBrush(EntityGroup& group, Vector3 pos){
    EntityID id = group.AddEntity();

    Sprite sprite = Sprite({pos.x, pos.y});
    Texture texture = RequestPlaceholderTexture();
    sprite.SetTexture(texture);
    sprite.SetSize(Session.gridSize,Session.gridSize);
    sprite.SetCenter(pos.x,pos.y);
    sprite.Hide();
    group.AddEntityComponent(COMP_SPRITE, id, sprite);

    PhysicsBody body = PhysicsBody(false);
    group.AddEntityComponent(COMP_PHYS_BODY,id,body);

    return id;
}

// session contructor
EditorSession::EditorSession() {
        REGCOMP(COMP_SPRITE,            DescribeComponentSprite);
        REGCOMP(COMP_ANIM_PLAYER,       DescribeComponentAnimationPlayer);
        REGCOMP(COMP_PLAT_PLAYER,       DescribeComponentPlatformerPlayer);
        REGCOMP(COMP_PHYS_BODY,         DescribeComponentPhysicsBody);

#ifdef MAGMA_3D
        REGCOMP(COMP_BASE,              DescribeComponentBase);
        REGCOMP(COMP_MODEL_RENDERER,    DescribeComponentModelRenderer);
#endif

        // declare stock builder-functions
        RegisterStockEntityBuilder(SpawnWallBrush);
}

// utils

// HACK: We need to add an artificial delay when switching editor modes 
// to avoid insta-clicking newly opened popup-menus
// Instead of improving the popup code, this hack is used instead.
static float NextModeTime = 0.f;
static EditorMode NextMode = MODE_NORMAL;

static void SwitchMode(EditorMode mode){
    Session.mode = MODE_DELAY;
    NextMode = mode;
    NextModeTime = GetTime() + 0.1f;
    DEBUG("Editor switched mode");
}

static void DrawGridCell(Vector2 worldPos, float size, float thick=1.f, Color tint=GRAY){
    Vector2 pos = Vector2Snap(worldPos, size);
    Rectangle cell = { pos.x, pos.y , size, size };
    DrawRectangleLinesEx(cell, thick, tint);
}

static void DrawGrid(Camera2D camera){
    Vector2 mouse = GetWindowMousePosition(camera);

    // draw cursor grid cell
    DrawGridCell(mouse, Session.gridSize);

    Color col = fabs(Session.gridSize - PIXELS_PER_UNIT) < EPSILON ? ColorAlpha(WHITE,0.5f) : ColorAlpha(LIGHTGRAY,0.5f); 
    for (int y = -3; y < 3; y++){
        for (int x = -3; x < 3; x++){
            Vector2 offset = { x, y };
            Vector2 cellPos = Vector2Add(mouse,Vector2Add(Vector2Scale(offset,Session.gridSize),offset));
            DrawGridCell(cellPos, Session.gridSize, 1.f, col);
        }
    }
}

static void DrawAxis(Vector2 pos, float len=20.f, float thick=2.f){
    DrawLineEx(pos, {pos.x + len, pos.y}, thick, RED);
    DrawLineEx(pos, {pos.x, pos.y + len}, thick, GREEN);
}
