# Create Your Profile - Frontend

Static HTML/CSS implementation of the "Create Your Profile" page with desktop and mobile breakpoints.

## Preview locally

```bash
# From /workspace
python3 -m http.server 8080
# open http://localhost:8080
```

## Structure

- `index.html` — semantic HTML5 structure
- `styles.css` — responsive styles (desktop + mobile)
- `assets/logo.svg` — placeholder logo

## Tech Notes

- Font: Inter (Google Fonts) with fallbacks
- Layout: Flexbox + CSS Grid
- Colors, spacing, and typography follow the spec
- No JavaScript; selects/checkbox styled via CSS only

## Deploy options

### GitHub Pages
1. Create a new repo and push these files to the repository root
2. In repo Settings → Pages → Build and deployment → Deploy from a branch
3. Branch: `main`, Folder: `/ (root)` → Save
4. Your site will be available at `https://<username>.github.io/<repo>/`

### Netlify
1. Drag-and-drop the folder on Netlify Dashboard, or connect Git repo
2. Build command: none; Publish directory: `/`
3. Deploy. Netlify will host at a generated URL which you can customize

### CodePen
1. Create a new Pen
2. Copy `index.html` body into HTML panel, `styles.css` into CSS panel
3. Add Inter font in Settings → HTML → Stuff for `<head>`

## License
MIT