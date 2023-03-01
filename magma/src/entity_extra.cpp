#include "magma.h"

// TODO: make methods member of entitygroup somehow
static AdvEntityGroup* Group = NULL;

// contains more "advanced" components and entities, not needed for some types of games
// can be removed from engine

// component-independent entity functions
void TranslateEntity(EntityID id, Vector2 offset){
    assert(Group);

    PhysicsBody* phys = NULL;
    if (Group->TryGetEntityComponent(id, COMP_PHYS_BODY, &phys)) {
        b2Vec2 pos = phys->body->GetPosition();
        phys->body->SetTransform({pos.x + offset.x/PIXELS_PER_UNIT,
                                  pos.y + offset.y/PIXELS_PER_UNIT},0.f);
    }
    else {
        Sprite* sprite = NULL;
        Group->GetEntityComponent(id, COMP_SPRITE, &sprite);
        sprite->Translate(offset);
    }
}

#ifdef MAGMA_3D
void TranslateEntity(EntityID id, Vector3 offset){
    assert(Group);

    Base* base = NULL;
    Group->GetEntityComponent(id, COMP_BASE);

    base->Translate(offset);
}
#endif

void SetEntityCenter(EntityID id, Vector2 pos){
    assert(Group);

    PhysicsBody* phys = NULL;
    if (Group->TryGetEntityComponent(id, COMP_PHYS_BODY, &phys)) {
        if (phys->initialized) {
            phys->body->SetTransform({ pos.x / PIXELS_PER_UNIT,pos.y / PIXELS_PER_UNIT }, 0.f);
        }
    }
    else {
        Sprite* sprite = NULL;
        Group->GetEntityComponent(id, COMP_SPRITE, &sprite);

        sprite->SetCenter(pos);
    }
}

#ifdef MAGMA_3D
void SetEntityCenter(EntityID id, Vector3 pos){
    assert(Group);

    Base* base = NULL;
    Group->GetEntityComponent(id, COMP_BASE, base);

    base->SetCenter(pos);
}
#endif

void SetEntitySize(EntityID id, Vector2 size){
    assert(Group);

    PhysicsBody* phys = NULL;
    if (Group->TryGetEntityComponent(id, COMP_PHYS_BODY, &phys)) {
        // TODO:
        b2Fixture* fixture = phys->body->GetFixtureList();
        if(fixture){
            const b2Shape* shape = fixture->GetShape();
            switch (shape->GetType()){
                case b2Shape::e_polygon:
                    {
                        auto poly = (b2PolygonShape*) shape;
                        b2Vec2 pos = phys->body->GetPosition();
                        poly->SetAsBox(size.x*0.5f, size.y*0.5f, pos, 0.f);
                        // TODO: test
                    }
                    break;
                default:
                    ERROR("TODO: Other shapes not implemented");
                    break;
            }
        }else {
            ERROR("Physics object does not have a single fixture with a shape");
        }
    }
    else
    {
        Sprite* sprite = NULL;
        Group->GetEntityComponent(id, COMP_SPRITE, &sprite);

        sprite->SetSize(size);
    }
}

#ifdef MAGMA_3D
void SetEntitySize(EntityID id, Vector3 pos){
    assert(Group);

    Base* base = NULL;
    Group->GetEntityComponent(id, COMP_BASE, base);

    base->SetSize(pos);
}
#endif

void SimplifyHitboxes(EntityGroup& group){
    ERROR("NOT IMPLEMENTED");
}

PhysicsBody::PhysicsBody(float density, float friction){
    this->initialized = false;
    this->dynamic = true;
    this->density = density;
    this->friction = friction;
}
PhysicsBody::PhysicsBody(bool isDynamic) {
    this->initialized = false;
    this->dynamic = isDynamic;
    this->density = 0.f;
    this->friction = 0.f;
}

PhysicsBody::~PhysicsBody(){
}

Vector2 PhysicsBody::position(){
    Vector2 result = {};
    if (initialized){
        assert(body);
        auto pos = body->GetPosition();
        result.x = pos.x;
        result.y = pos.y;
    }
    return result;
}

float PhysicsBody::angle(){
    float angle = 0.f;
    if (initialized){
        assert(body);
        angle = body->GetAngle();
    }
    return angle;
}

PlatformerPlayer::PlatformerPlayer(float moveSpeed, float jumpForce, PlayerPose defaultPose) {
    this->moveSpeed = moveSpeed;
    this->jumpForce = jumpForce;
    this->pose = defaultPose;
}

AnimationPlayer::AnimationPlayer()
    : curFrame(0), curAnim(NULL), timer(0.f) {
}

AnimationPlayer::AnimationPlayer(const SheetAnimation& startAnim)
    : curFrame(0), curAnim(&startAnim), timer(0.f) {
}

void AnimationPlayer::SetAnimation(const SheetAnimation& anim) {
    // don't change animation if it already is playing
    if (curAnim == NULL || curAnim->name != anim.name) {
        curAnim = &anim;
        timer = 0.f;
        curFrame = 0;
    }
}

AdvEntityGroup::AdvEntityGroup(float gravity)
    : EntityGroup() {
    this->gravity = gravity;
    this->world = NULL;

    ClearGroup();

    // add extended updaters, drawers
    RegisterUpdater(UpdateExtendedComponent);
}

AdvEntityGroup::~AdvEntityGroup(){
    delete this->world;
    DEBUG("Disposed Box2D world");
}

void AdvEntityGroup::ClearGroup() {
    delete this->world;

    // make box2d world
    b2Vec2 gravVec2(0.f,gravity);
    this->world = new b2World(gravVec2);
    DEBUG("Allocated Box2D world");

    EntityGroup::ClearGroup();
}

void AdvEntityGroup::UpdateGroup(float delta){
    EntityGroup::UpdateGroup(delta);
    Group = this;

    // TODO: make optional
    // run physics engine
    float timeStep = delta;
    int velocityIterations = 6;
    int positionIterations = 2;

    assert(world);
    world->Step(timeStep, velocityIterations, positionIterations);
}

void AdvEntityGroup::DrawGroupDebug(){
    EntityGroup::DrawGroupDebug();
    UpdateAndRenderInteractiveGroupLoader();
}

static std::string ParsePath(const char* file, const char* folder = NULL) {
    if (file == NULL || TextIsEqual(file, "")) {
        return "";
    }

    std::string output = "";

    // set extension
    auto path = std::filesystem::path(file);
    path.replace_extension("comps");

    // add map prefix
    auto newName = "map_" + path.filename().string();
    path.replace_filename(newName);

    // add folder before if any
    if (folder != NULL && !TextIsEqual(folder, "")) {
        path = std::filesystem::path(folder) / path;
    }

    return path.string();
}


static EntityGroup* InteractiveEntityGroup = NULL;
static const char* InteractiveFolder = NULL;
static uint InteractiveVersion = 0;
static void SaveGroupInteractivelyCallback(std::string& text) {
    // replace spaces with underscores
    std::replace(text.begin(), text.end(), ' ', '_');

    // make name always lowercase
    text = TextToLower(text.c_str());

    std::string outputStr = ParsePath(text.c_str(), InteractiveFolder);
    if (outputStr.empty()) {
        WARN("EntityGroup interactive save cancelled");
    }
    else {
        InteractiveEntityGroup->SaveGroup(outputStr.c_str(), InteractiveVersion);
    }
}

void AdvEntityGroup::SaveGroupInteractively(const char* folder, uint version) {
    InteractiveFolder = folder;
    InteractiveEntityGroup = this;
    InteractiveVersion = version;
    if (!ShowInputBox("Save entity group", &SaveGroupInteractivelyCallback, "mylevel", 4, 32)) {
        ERROR("Another input box is already opened!");
    }
}

static void CollectSavedGroups(std::vector<std::string>* groups) {
    std::vector<std::string> searchPaths = {
        ".",
        "./raw_assets",
        "./temple/raw_assets",
        "../../temple/raw_assets",
        "../../raw_assets",
    };

    for (const auto& path : searchPaths) {
        FilePathList list = LoadDirectoryFilesEx(path.c_str(), ".comps", true);
        for (int i = 0; i < list.count; i++) {
            auto path = list.paths[i];
            groups->push_back(path);
            DEBUG("Found map at %s.", path);
        }
        UnloadDirectoryFiles(list);
    }
}

static std::vector<std::string> InteractiveGroups;
static PopMenu InteractivePopMenu = PopMenu(FOCUS_CRITICAL);
void AdvEntityGroup::LoadGroupInteractively(uint version) {
    // TODO: put in entity_extra maybe
    InteractiveVersion = version;
    InteractiveGroups.clear();
    CollectSavedGroups(&InteractiveGroups);
}

void AdvEntityGroup::UpdateAndRenderInteractiveGroupLoader() {
    if (InteractiveGroups.empty()) {
        return;
    }

    auto& menu = InteractivePopMenu;
    menu.RenderPanel();

    for (const auto& group : InteractiveGroups) {
        // TODO: check versions
        std::string smaller = group;
        while (smaller.size() > 32){
            smaller.pop_back();
        }
        const char* displayName = TextFormat("v%d - %s", 0, smaller.c_str());
        menu.DrawPopButton(displayName);
    }

    menu.DrawPopButton("-- Cancel");
    menu.EndButtons();

    int index = 0;
    if (menu.IsButtonSelected(&index)) {
        if (index >= 0 && index < InteractiveGroups.size()) {
            auto path = InteractiveGroups[index].c_str();
            INFO("Interactively loading level %s...", path);
            LoadGroup(path);
        }
        InteractiveGroups.clear();
    }
}


void UpdateExtendedComponent(EntityGroup& _group, IteratedComp& comp, float delta){
    AdvEntityGroup& group = *(AdvEntityGroup*) &_group; // eww

    switch (comp.second.type) {
        case COMP_ANIM_PLAYER:
        {
            auto animPlayer = (AnimationPlayer*)comp.second.data;

            Sprite* sprite = NULL;
            group.GetEntityComponent(comp.first, COMP_SPRITE, &sprite);

            // advance current animation
            const SheetAnimation& anim = *animPlayer->curAnim;

            // check if not playing outside of sheet
            // TODO: FIX CRITICAL MEMORY BUG
            //assert(abs(animPlayer->curFrame) < anim.count);

            // set current frame
            Texture sheetTexture = RequestIndexedTexture(anim.sheetName);
            int absFrame = abs(animPlayer->curFrame);
            if (animPlayer->curFrame > anim.count) absFrame = 0; // TODO: hack
            Rectangle src = {
                anim.origin.x + anim.cellSize.x * absFrame,
                anim.origin.y,
                anim.cellSize.x,
                anim.cellSize.y,
            };

            sprite->SetTexture(sheetTexture,src);

            if (animPlayer->timer > 1.f/anim.fps){
                animPlayer->timer = 0.f;
                animPlayer->curFrame++;

                switch (anim.mode){
                    case PLAY_LOOP:
                        animPlayer->curFrame = Wrap(animPlayer->curFrame,0,anim.count-1);
                        break;
                    case PLAY_ONCE:
                        animPlayer->curFrame = Clamp(animPlayer->curFrame,0,anim.count-1);
                        break;
                    case PLAY_PING_PONG:
                        if (animPlayer->curFrame >= (int)anim.count){
                            animPlayer->curFrame = -anim.count+1;
                        }
                        break;
                }
            }
            animPlayer->timer += delta;
        } break;
        case COMP_PHYS_BODY: 
        {
            PhysicsBody* phys = (PhysicsBody*) comp.second.data;
            Sprite* sprite = NULL;
            group.GetEntityComponent(comp.first, COMP_SPRITE, &sprite);

            // init if newly created
            if (!phys->initialized){
                b2BodyDef bodyDef;
                bodyDef.position.Set(sprite->bounds.min.x/PIXELS_PER_UNIT,
                                     sprite->bounds.min.y/PIXELS_PER_UNIT);

                if (phys->dynamic) {
                    bodyDef.type = b2_dynamicBody;
                }

                phys->body = group.world->CreateBody(&bodyDef);
                phys->body->SetFixedRotation(true);
                phys->body->SetSleepingAllowed(false);

                b2PolygonShape boxShape;
                boxShape.SetAsBox(sprite->halfSize().x/PIXELS_PER_UNIT,
                                  sprite->halfSize().y/PIXELS_PER_UNIT);

                if (phys->dynamic) {
                    b2FixtureDef fixtureDef;
                    fixtureDef.shape = &boxShape;
                    fixtureDef.density = 30.0f;
                    fixtureDef.friction = .8f;
                    phys->body->CreateFixture(&fixtureDef);
                }
                else {
                    phys->body->CreateFixture(&boxShape, 0.f);
                }

                phys->initialized = true;
            }

            // place sprite at PhysicsBody location
            Vector2 newPos = Vector2Scale(phys->position(),PIXELS_PER_UNIT);
            sprite->SetCenter(newPos);

        } break;
        case COMP_PLAT_PLAYER:
        {
            auto plat = (PlatformerPlayer*) comp.second.data;

            PhysicsBody* phys = NULL;
            group.GetEntityComponent(comp.first,COMP_PHYS_BODY, &phys);
            if (!phys->initialized) {
                break;
            }

            phys->body->SetGravityScale(3.f); // TODO: make field

            bool isMoving = false;
            b2Vec2 force = { 0.f, -0.01f };
            if (IsKeyDown(KEY_A)){
                force.x = -plat->moveSpeed;
                plat->pose = POSE_WALK;
                plat->isLookingRight = false;
                isMoving = true;
            }
            if (IsKeyDown(KEY_D)){
                force.x = plat->moveSpeed;
                plat->pose = POSE_WALK;
                plat->isLookingRight = true;
                isMoving = true;
            }
            if (IsKeyPressed(KEY_SPACE)){ // jump
                b2Vec2 jumpForce = { 0.f, -plat->jumpForce };
                phys->body->ApplyLinearImpulseToCenter(jumpForce,true);
                plat->pose = POSE_JUMP;
            }
            phys->body->ApplyForceToCenter(force,true);

            // TODO: take gravity into account
            // determine if falling or jumping
            PlayerPose* pose = &plat->pose;
            auto curVel = phys->body->GetLinearVelocity();

            if (curVel.y > EPSILON){ // dermine if falling
                plat->pose = POSE_FALL;
            }
            else if (curVel.y < -EPSILON){ // determine if jumping
                plat->pose = POSE_JUMP;
            }
            else if (!isMoving && abs(curVel.x) > 0.f){ // determine if sliding
                plat->pose = POSE_SLIDE;
            }
            else if (!isMoving) { // determine if idling
                plat->pose = POSE_IDLE;
            }

        } break;
    }
}

// ===== 3d =====
#ifdef MAGMA_3D

PlayerFPS::PlayerFPS(float eyeHeight) {
    this->eyeHeight = eyeHeight;
    this->isFocused = true;

    Camera* cam = &this->camera;
    cam->projection = CAMERA_PERSPECTIVE;
    cam->up = { 0.0f, 1.0f, 0.f };

    SetCameraMode(*cam, CAMERA_CUSTOM);

    SetAngle(0.f);
    SetFov(80.f);

    Focus();
}

void PlayerFPS::SetAngle(float lookAtDeg) {
    Vector3 pos = camera.position;
    Vector3 offset = {
        cosf(lookAtDeg * DEG2RAD),
        sinf(lookAtDeg * DEG2RAD),
        0
    };
    camera.target = Vector3Add(pos, offset);
}

void PlayerFPS::SetFov(float fovDeg) {
    camera.fovy = fovDeg;
}

// TODO shouldnt be a EntityGroup pointer but that doesnt compile for some reason
Vector3 PlayerFPS::Update(void* group, float delta) {

    // snap to the floor 
    Ray ray = { 0 };

    Vector3 offset = { 0,this->eyeHeight,0 };
    ray.position = Vector3Add(this->camera.position, offset);
    ray.direction = { 0,-1,0 };

    RayCollision col = ((EntityGroup*)group)->GetRayCollision(ray);

    // move player to hit point
    feet = Vector3Add(col.point, offset);
    camera.position = this->feet;

    if (this->isFocused) {
        // TODO naive use proper quaternions instead!

        Vector2 mouse = Vector2Scale(Vector2Scale(GetMouseDelta(), delta), 3.f);
        tilt -= mouse.y;
        angle -= mouse.x;
        SetMousePosition(GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f);

        // look around
        Vector3 angle = {
            sinf(this->angle * DEG2RAD),
            sinf(this->tilt * DEG2RAD) * 2,
            cosf(this->angle * DEG2RAD)
        };

        // clamp
        tilt = Clamp(tilt, -90, 90);

        // movement
        float axisX = 0;
        bool pressed = false;
        if (IsKeyDown(KEY_W)) {
            axisX = 90;
            pressed = true;
        }
        if (IsKeyDown(KEY_S)) {
            axisX = 270;
            pressed = true;
        }
        if (pressed) {
            Vector3 offset = { cosf((this->angle - axisX) * DEG2RAD), 0, sinf((this->angle + axisX) * DEG2RAD) };
            camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Scale(offset, delta), 10));
        }

        // look
        camera.target = Vector3Add(camera.position, angle);
    }

    // DrawText(TextFormat("%f\n%f\n\n%f\n%f\n%f horizontal movement not implemented lmao",player->angle,player->tilt,
    //                                                                        player->camera.position.x,
    //                                                                        player->camera.position.y,

    return col.point;
}

void PlayerFPS::Focus() {
    isFocused = true;
    SetCameraMode(camera, CAMERA_CUSTOM);
}

void PlayerFPS::Unfocus() {
    isFocused = false;
}

void PlayerFPS::Teleport(Vector3 pos) {
    camera.position = pos;
}

#endif
