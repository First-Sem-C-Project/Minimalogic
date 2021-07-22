SDL_Texture* FontTexture(SDL_Renderer *renderer, const char* message){
    TTF_Font* sans = TTF_OpenFont("fonts/sans.ttf", 50);
    if(sans == NULL){
        printf("Failed to load the font: %s\n", TTF_GetError());
        return NULL;
    }
    else{
        SDL_Surface* textSurface = NULL;
        SDL_Texture* textTexture = NULL;
        SDL_Color white = {WHITE, 200};
        textSurface = TTF_RenderText_Solid(sans, message, white);

        if(textSurface == NULL){
            printf("Failed to load the surface\n");
        }
        else{
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if(textTexture == NULL){
                printf("Failed to load the texture: %s\n",SDL_GetError());
            }
            SDL_FreeSurface(textSurface);
        }
        return textTexture;
    }
}

void RenderGateText(SDL_Renderer *renderer, SDL_Rect compo, Type type){
    SDL_Rect textRect = {compo.x+compo.w/4, compo.y+compo.h/4, compo.w/2, compo.h/2};
    switch (type){
        case(g_and):
            SDL_RenderCopy(renderer, FontTexture(renderer, "AND"), NULL, &textRect);
            break;
        case(g_or):
            SDL_RenderCopy(renderer, FontTexture(renderer, "OR"), NULL, &textRect);
            break;
        case(g_nand):
            SDL_RenderCopy(renderer, FontTexture(renderer, "NAND"), NULL, &textRect);            
            break;
        case(g_nor):
            SDL_RenderCopy(renderer, FontTexture(renderer, "NOR"), NULL, &textRect);           
            break;
        case(g_xor):
            SDL_RenderCopy(renderer, FontTexture(renderer, "XOR"), NULL, &textRect);            
            break;
        default:
            SDL_RenderCopy(renderer, FontTexture(renderer, "XNOR"), NULL, &textRect);            
            break;
    }
}
