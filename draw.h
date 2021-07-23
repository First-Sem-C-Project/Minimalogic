SDL_Texture* FontTexture(SDL_Renderer *renderer, const char* message){
    TTF_Font* sans = TTF_OpenFont("fonts/sans.ttf", 50);
    SDL_Surface* textSurface = NULL;
    SDL_Texture* textTexture = NULL;
    if(sans == NULL){
        printf("Failed to load the font: %s\n", TTF_GetError());
    }
    else{
        SDL_Color white = {WHITE, 200};
        textSurface = TTF_RenderText_Solid(sans, message, white);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    }

    SDL_FreeSurface(textSurface);
    TTF_CloseFont(sans);
    return textTexture;
}

void RenderGateText(SDL_Renderer *renderer, SDL_Rect compo, Type type){
    SDL_Rect textRect = {compo.x+compo.w/4, compo.y+compo.h/4, compo.w/2, compo.h/2};
    SDL_Texture* text = NULL;
    switch (type){
        case(g_and):
            text = FontTexture(renderer, "AND");
            SDL_RenderCopy(renderer, text, NULL, &textRect);
            break;
        case(g_or):
            text = FontTexture(renderer, "OR");
            SDL_RenderCopy(renderer, text, NULL, &textRect);
            break;
        case(g_nand):
            text = FontTexture(renderer, "NAND");
            SDL_RenderCopy(renderer, text, NULL, &textRect);            
            break;
        case(g_nor):
            text = FontTexture(renderer, "NOR");
            SDL_RenderCopy(renderer, text, NULL, &textRect);           
            break;
        case(g_xor):
            text = FontTexture(renderer, "XOR");
            SDL_RenderCopy(renderer, text, NULL, &textRect);            
            break;
        default:
            text = FontTexture(renderer, "XNOR");
            SDL_RenderCopy(renderer, text, NULL, &textRect);            
            break;
    }
    SDL_DestroyTexture(text);
}