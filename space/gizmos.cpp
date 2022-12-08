const Vector3 SUN_GIZMO_POS = {3, 3, 3};

void gizmos_draw(LevelFeed *feed)
{
    // draw light icons
    DrawLine3D(SUN_GIZMO_POS, Vector3Add(SUN_GIZMO_POS, feed->environment.sunDirection), WHITE);

    for (int i = 0; i < feed->lamps.count; i++)
    {
        Lamp *lamp = feed->lamps.get(i);
        DrawPoint3D(lamp->pos, lamp->color);
    }
}