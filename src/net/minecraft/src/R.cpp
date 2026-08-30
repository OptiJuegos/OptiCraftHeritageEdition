// RenderLightningBolt implementation moved to RenderLightningBolt.cpp
#if 0
RenderLightningBolt::RenderLightningBolt() {
}

void RenderLightningBolt::doRenderLightningBolt(EntityLightningBolt* entitylightningbolt, double d, double d1, double d2, float f, float f1) {
    Tessellator* tessellator = &Tessellator::instance;
    renderDisable(RenderCapability::Texture2D);
    renderDisable(RenderCapability::Lighting);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::One);
    double ad[8];
    double ad1[8];
    double d3 = 0.0;
    double d4 = 0.0;
    Random random(entitylightningbolt->field_27029_a);
    for (int j = 7; j >= 0; j--) {
        ad[j] = d3;
        ad1[j] = d4;
        d3 += random.nextInt(11) - 5;
        d4 += random.nextInt(11) - 5;
    }

    for (int i = 0; i < 4; i++) {
        Random random1(entitylightningbolt->field_27029_a);
        for (int k = 0; k < 3; k++) {
            int l = 7;
            int i1 = 0;
            if (k > 0) {
                l = 7 - k;
            }
            if (k > 0) {
                i1 = l - 2;
            }
            double d5 = ad[l] - d3;
            double d6 = ad1[l] - d4;
            for (int j1 = l; j1 >= i1; j1--) {
                double d7 = d5;
                double d8 = d6;
                if (k == 0) {
                    d5 += random1.nextInt(11) - 5;
                    d6 += random1.nextInt(11) - 5;
                } else {
                    d5 += random1.nextInt(31) - 15;
                    d6 += random1.nextInt(31) - 15;
                }
                tessellator->startDrawing(5);
                float f2 = 0.5f;
                tessellator->setColorRGBA_F(0.9f * f2, 0.9f * f2, 1.0f * f2, 0.3f);
                double d9 = 0.1 + (double)i * 0.2;
                if (k == 0) {
                    d9 *= (double)j1 * 0.1 + 1.0;
                }
                double d10 = 0.1 + (double)i * 0.2;
                if (k == 0) {
                    d10 *= (double)(j1 - 1) * 0.1 + 1.0;
                }
                for (int k1 = 0; k1 < 5; k1++) {
                    double d11 = (d + 0.5) - d9;
                    double d12 = (d2 + 0.5) - d9;
                    if (k1 == 1 || k1 == 2) {
                        d11 += d9 * 2.0;
                    }
                    if (k1 == 2 || k1 == 3) {
                        d12 += d9 * 2.0;
                    }
                    double d13 = (d + 0.5) - d10;
                    double d14 = (d2 + 0.5) - d10;
                    if (k1 == 1 || k1 == 2) {
                        d13 += d10 * 2.0;
                    }
                    if (k1 == 2 || k1 == 3) {
                        d14 += d10 * 2.0;
                    }
                    tessellator->addVertex(d13 + d5, d1 + (double)(j1 * 16), d14 + d6);
                    tessellator->addVertex(d11 + d7, d1 + (double)((j1 + 1) * 16), d12 + d8);
                }
                tessellator->draw();
            }
        }
    }

    renderDisable(RenderCapability::Blend);
    renderEnable(RenderCapability::Lighting);
    renderEnable(RenderCapability::Texture2D);
}

void RenderLightningBolt::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderLightningBolt((EntityLightningBolt*)entity, d, d1, d2, f, f1);
}
#endif
