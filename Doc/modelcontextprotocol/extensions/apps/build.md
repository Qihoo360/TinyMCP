> ## Documentation Index
> Fetch the complete documentation index at: https://modelcontextprotocol.io/llms.txt
> Use this file to discover all available pages before exploring further.

# Build an MCP App

> Getting started guide for building interactive UI applications with MCP Apps

## Prerequisites

You'll need [Node.js](https://nodejs.org/en/download) 20 or higher. Familiarity
with [MCP tools](https://modelcontextprotocol.io/specification/latest/server/tools.md) and
[resources](https://modelcontextprotocol.io/specification/latest/server/resources.md) is recommended since MCP
Apps combine both primitives. Experience with the
[MCP TypeScript SDK](https://github.com/modelcontextprotocol/typescript-sdk)
will help you better understand the server-side patterns.

## Getting started

The fastest way to create an MCP App is using an AI coding agent with the MCP
Apps skill. If you prefer to set up a project manually, skip to
[Manual setup](#manual-setup).

### Using an AI coding agent

AI coding agents with Skills support can scaffold a complete MCP App project for
you. Skills are folders of instructions and resources that your agent loads when
relevant. They teach the AI how to perform specialized tasks like creating MCP
Apps.

The `create-mcp-app` skill includes architecture guidance, best practices, and
working examples that the agent uses to generate your project.

**1. Install the skill**

If you are using Claude Code, you can install the skill directly with:

```
/plugin marketplace add modelcontextprotocol/ext-apps
/plugin install mcp-apps@modelcontextprotocol-ext-apps
```

You can also use the [Vercel Skills CLI](https://skills.sh/) to install skills across different AI coding agents:

```bash theme={null}
npx skills add modelcontextprotocol/ext-apps
```

Alternatively, you can install the skill manually by cloning the ext-apps repository:

```bash theme={null}
git clone https://github.com/modelcontextprotocol/ext-apps.git
```

And then copying the skill to the appropriate location for your agent:

| Agent | Skills directory (macOS/Linux) | Skills directory (Windows) |
| - | - | - |
| [Claude Code](https://docs.anthropic.com/en/docs/claude-code/skills) | `~/.claude/skills/` | `%USERPROFILE%\.claude\skills\` |
| [VS Code](https://code.visualstudio.com/docs/copilot/customization/agent-skills) and [GitHub Copilot](https://docs.github.com/en/copilot/concepts/agents/about-agent-skills) | `~/.copilot/skills/` | `%USERPROFILE%\.copilot\skills\` |
| [Gemini CLI](https://geminicli.com/docs/cli/skills/) | `~/.gemini/skills/` | `%USERPROFILE%\.gemini\skills\` |
| [Cline](https://cline.bot/blog/cline-3-48-0-skills-and-websearch-make-cline-smarter) | `~/.cline/skills/` | `%USERPROFILE%\.cline\skills\` |
| [Goose](https://goose-docs.ai/docs/guides/context-engineering/using-skills/) | `~/.config/goose/skills/` | `%USERPROFILE%\.config\goose\skills\` |
| [Codex](https://developers.openai.com/codex/skills/) | `~/.codex/skills/` | `%USERPROFILE%\.codex\skills\` |
| [Cursor](https://cursor.com/docs/context/skills) | `~/.cursor/skills/` | `%USERPROFILE%\.cursor\skills\` |

> [!NOTE]
>
> This list is not comprehensive. Other agents may support skills in different locations; check your agent's documentation.

For example, with Claude Code you can install the skill globally (available in all projects):

```bash macOS/Linux theme={null}
cp -r ext-apps/plugins/mcp-apps/skills/create-mcp-app ~/.claude/skills/create-mcp-app
```

```powershell Windows theme={null}
Copy-Item -Recurse ext-apps\plugins\mcp-apps\skills\create-mcp-app $env:USERPROFILE\.claude\skills\create-mcp-app
```

Or install it for a single project only by copying to `.claude/skills/` in your project directory:

```bash macOS/Linux theme={null}
mkdir -p .claude/skills && cp -r ext-apps/plugins/mcp-apps/skills/create-mcp-app .claude/skills/create-mcp-app
```

```powershell Windows theme={null}
New-Item -ItemType Directory -Force -Path .claude\skills | Out-Null; Copy-Item -Recurse ext-apps\plugins\mcp-apps\skills\create-mcp-app .claude\skills\create-mcp-app
```

To verify the skill is installed, ask your agent "What skills do you have access to?" — you should see `create-mcp-app` as one of the available skills.

**2. Create your app**

Ask your AI coding agent to build it:

```
Create an MCP App that displays a color picker
```

The agent will recognize the `create-mcp-app` skill is relevant, load its instructions, then scaffold a complete project with server, UI, and configuration files.

![Creating a new MCP App with Claude Code](https://mintcdn.com/mcp/GU_E-622SLWFdCrP/images/quickstart-apps/create-mcp-app-skill.gif?s=6c3a3b8a7590b5e97b5c3d8480a9ab12)

Creating a new MCP App with Claude Code

**3. Run your app**

```bash macOS/Linux theme={null}
npm install && npm run build && npm run serve
```

```powershell Windows theme={null}
npm install; npm run build; npm run serve
```

> [!TIP]
>
> You might need to make sure that you are first in the **app folder** before running the commands above.

**4. Test your app**

Follow the instructions in [Testing your app](#testing-your-app) below. For the color picker example, start a new chat and ask Claude to provide you a color picker.

![Testing the color picker in Claude](https://mintcdn.com/mcp/GU_E-622SLWFdCrP/images/quickstart-apps/test-color-picker.gif?s=09413b99bc31d7edc7f9aa22df4faa6a)

Testing the color picker in Claude

### Manual setup

If you're not using an AI coding agent, or prefer to understand the setup
process, follow these steps.

**1. Create the project structure**

A typical MCP App project separates the server code from the UI code:

**my-mcp-app**

**package.json**

**tsconfig.json**

**vite.config.ts**

**server.ts**

**mcp-app.html**

**src**

**mcp-app.ts**

The server registers the tool and serves the UI resource. The UI resource will eventually be rendered in a secure iframe with deny-by-default CSP configuration. If your app has CSS and JS assets, you will need to [configure CSP](https://apps.extensions.modelcontextprotocol.io/api/documents/Patterns.html#configuring-csp-and-cors), or you can bundle your assets into the HTML with a tool like `vite-plugin-singlefile`, which is what we will do in this tutorial.

**2. Install dependencies**

```bash theme={null}
npm install @modelcontextprotocol/ext-apps @modelcontextprotocol/sdk
npm install -D typescript vite vite-plugin-singlefile express cors @types/express @types/cors tsx
```

The `ext-apps` package provides helpers for both the server side (registering tools and resources) and the client side (the `App` class for UI-to-host communication). Vite with the `vite-plugin-singlefile` plugin is used here to bundle your UI and assets into a single HTML file for convenience, but this is optional — you can use any bundler or serve unbundled files if you [configure CSP](https://apps.extensions.modelcontextprotocol.io/api/documents/Patterns.html#configuring-csp-and-cors).

**3. Configure the project**

> [tab] package.json
>
> The `"type": "module"` setting enables ES module syntax. The `build` script uses the `INPUT` environment variable to tell Vite which HTML file to bundle. The `serve` script runs your server using `tsx` for TypeScript execution.
>
> ```json theme={null}
> {
>   "type": "module",
>   "scripts": {
>     "build": "INPUT=mcp-app.html vite build",
>     "serve": "npx tsx server.ts"
>   }
> }
> ```

> [tab] tsconfig.json
>
> The TypeScript configuration targets modern JavaScript (`ES2022`) and uses ESNext modules with bundler resolution, which works well with Vite. The `include` array covers both the server code in the root and UI code in `src/`.
>
> ```json theme={null}
> {
>   "compilerOptions": {
>     "target": "ES2022",
>     "module": "ESNext",
>     "moduleResolution": "bundler",
>     "strict": true,
>     "esModuleInterop": true,
>     "skipLibCheck": true,
>     "outDir": "dist"
>   },
>   "include": ["*.ts", "src/**/*.ts"]
> }
> ```

> [tab] vite.config.ts
>
> ```typescript theme={null}
> import { defineConfig } from "vite";
> import { viteSingleFile } from "vite-plugin-singlefile";
>
> export default defineConfig({
>   plugins: [viteSingleFile()],
>   build: {
>     outDir: "dist",
>     rollupOptions: {
>       input: process.env.INPUT,
>     },
>   },
> });
> ```

**4. Build the project**

With the project structure and configuration in place, continue to [Building an MCP App](#building-an-mcp-app) below to implement the server and UI.

## Building an MCP App

Let's build a simple app that displays the current server time. This example
demonstrates the full pattern: registering a tool with UI metadata, serving the
bundled HTML as a resource, and building a UI that communicates with the server.

### Server implementation

The server needs to do two things: register a tool that includes the
`_meta.ui.resourceUri` field, and register a resource handler that serves the
bundled HTML. Here's the complete server file:

```typescript theme={null}
// server.ts
console.log("Starting MCP App server...");

import { McpServer } from "@modelcontextprotocol/sdk/server/mcp.js";
import { StreamableHTTPServerTransport } from "@modelcontextprotocol/sdk/server/streamableHttp.js";
import {
  registerAppTool,
  registerAppResource,
  RESOURCE_MIME_TYPE,
} from "@modelcontextprotocol/ext-apps/server";
import cors from "cors";
import express from "express";
import fs from "node:fs/promises";
import path from "node:path";

const server = new McpServer({
  name: "My MCP App Server",
  version: "1.0.0",
});

// The ui:// scheme tells hosts this is an MCP App resource.
// The path structure is arbitrary; organize it however makes sense for your app.
const resourceUri = "ui://get-time/mcp-app.html";

// Register the tool that returns the current time
registerAppTool(
  server,
  "get-time",
  {
    title: "Get Time",
    description: "Returns the current server time.",
    inputSchema: {},
    _meta: { ui: { resourceUri } },
  },
  async () => {
    const time = new Date().toISOString();
    return {
      content: [{ type: "text", text: time }],
    };
  },
);

// Register the resource that serves the bundled HTML
registerAppResource(
  server,
  resourceUri,
  resourceUri,
  { mimeType: RESOURCE_MIME_TYPE },
  async () => {
    const html = await fs.readFile(
      path.join(import.meta.dirname, "dist", "mcp-app.html"),
      "utf-8",
    );
    return {
      contents: [
        { uri: resourceUri, mimeType: RESOURCE_MIME_TYPE, text: html },
      ],
    };
  },
);

// Expose the MCP server over HTTP
const expressApp = express();
expressApp.use(cors());
expressApp.use(express.json());

expressApp.post("/mcp", async (req, res) => {
  const transport = new StreamableHTTPServerTransport({
    sessionIdGenerator: undefined,
    enableJsonResponse: true,
  });
  res.on("close", () => transport.close());
  await server.connect(transport);
  await transport.handleRequest(req, res, req.body);
});

expressApp.listen(3001, (err) => {
  if (err) {
    console.error("Error starting server:", err);
    process.exit(1);
  }
  console.log("Server listening on http://localhost:3001/mcp");
});
```

Let's break down the key parts:

* **`resourceUri`**: The `ui://` scheme tells hosts this is an MCP App resource.
  The path structure is arbitrary.
* **`registerAppTool`**: Registers a tool with the `_meta.ui.resourceUri` field.
  When the host calls this tool, the UI is fetched and rendered, and the tool result is passed to it upon arrival.
* **`registerAppResource`**: Serves the bundled HTML when the host requests the UI resource.
* **Express server**: Exposes the MCP server over HTTP on port 3001.

### UI implementation

The UI consists of an HTML page and a TypeScript module that uses the `App`
class to communicate with the host. Here's the HTML:

```html theme={null}
<!-- mcp-app.html -->
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>Get Time App</title>
  </head>
  <body>
    <p>
      <strong>Server Time:</strong>
      <code id="server-time">Loading...</code>
    </p>
    <button id="get-time-btn">Get Server Time</button>
    <script type="module" src="/src/mcp-app.ts"></script>
  </body>
</html>
```

And the TypeScript module:

```typescript theme={null}
// src/mcp-app.ts
import { App } from "@modelcontextprotocol/ext-apps";

const serverTimeEl = document.getElementById("server-time")!;
const getTimeBtn = document.getElementById("get-time-btn")!;

const app = new App({ name: "Get Time App", version: "1.0.0" });

// Establish communication with the host
app.connect();

// Handle the initial tool result pushed by the host
app.ontoolresult = (result) => {
  const time = result.content?.find((c) => c.type === "text")?.text;
  serverTimeEl.textContent = time ?? "[ERROR]";
};

// Proactively call tools when users interact with the UI
getTimeBtn.addEventListener("click", async () => {
  const result = await app.callServerTool({
    name: "get-time",
    arguments: {},
  });
  const time = result.content?.find((c) => c.type === "text")?.text;
  serverTimeEl.textContent = time ?? "[ERROR]";
});
```

The key parts:

* **`app.connect()`**: Establishes communication with the host. Call this once
  when your app initializes.
* **`app.ontoolresult`**: A callback that fires when the host pushes a tool
  result to your app (e.g., when the tool is first called and the UI renders).
* **`app.callServerTool()`**: Lets your app proactively call tools on the server.
  Keep in mind that each call involves a round-trip to the server, so design your
  UI to handle latency gracefully.

The `App` class provides additional methods for logging, opening URLs, and
updating the model's context with structured data from your app. See the full
[API documentation](https://apps.extensions.modelcontextprotocol.io/api/).

## Testing your app

To test your MCP App, build the UI and start your local server:

```bash macOS/Linux theme={null}
npm run build && npm run serve
```

```powershell Windows theme={null}
npm run build; npm run serve
```

In the default configuration, your server will be available at
`http://localhost:3001/mcp`. However, to see your app render, you need an MCP
host that supports MCP Apps. You have several options.

### Testing with Claude

[Claude](https://claude.ai) (web) and [Claude Desktop](https://claude.ai/download)
support MCP Apps. For local development, you'll need to expose your server to
the internet. You can run an MCP server locally and use tools like `cloudflared`
to tunnel traffic through.

In a separate terminal, run:

```bash theme={null}
npx cloudflared tunnel --url http://localhost:3001
```

Copy the generated URL (e.g., `https://random-name.trycloudflare.com`) and add it
as a [custom connector](https://support.anthropic.com/en/articles/11175166-getting-started-with-custom-connectors-using-remote-mcp)
in Claude - click on your profile, go to **Settings**, **Connectors**, and
finally **Add custom connector**.

> [!NOTE]
>
> Custom connectors are available on paid Claude plans (Pro, Max, or Team).

![Adding a custom connector in Claude](https://mintcdn.com/mcp/GU_E-622SLWFdCrP/images/quickstart-apps/add-custom-connector.gif?s=c4ec0750413ff7575c7f9492e2713212)

Adding a custom connector in Claude

### Testing with the basic-host

The `ext-apps` repository includes a test host for development. Clone the repo and
install dependencies:

```bash macOS/Linux theme={null}
git clone https://github.com/modelcontextprotocol/ext-apps.git
cd ext-apps/examples/basic-host
npm install
```

```powershell Windows theme={null}
git clone https://github.com/modelcontextprotocol/ext-apps.git
cd ext-apps\examples\basic-host
npm install
```

Running `npm start` from `ext-apps/examples/basic-host/` will start the basic-host
test interface. To connect it to a specific server (e.g., one you're developing),
pass the `SERVERS` environment variable inline:

```bash macOS/Linux theme={null}
SERVERS='["http://localhost:3001/mcp"]' npm start
```

```powershell Windows theme={null}
$env:SERVERS='["http://localhost:3001/mcp"]'; npm start
```

Navigate to `http://localhost:8080`. You'll see a simple interface where you can
select a tool and call it. When you call your tool, the host fetches the UI
resource and renders it in a sandboxed iframe. You can then interact with your
app and verify that tool calls work correctly.

![Example of the QR code MCP App running with the basic host](https://mintcdn.com/mcp/GU_E-622SLWFdCrP/images/quickstart-apps/qr-code-server.gif?s=48a3b47239b8394017c0949162d63de9)

Testing the QR code MCP App with the basic host

## Learn more

> [card] [API Documentation](https://apps.extensions.modelcontextprotocol.io/api/)
>
> Full SDK reference and API details

> [card] [GitHub Repository](https://github.com/modelcontextprotocol/ext-apps)
>
> Source code, examples, and issue tracker

> [card] [Specification](https://github.com/modelcontextprotocol/ext-apps/blob/main/specification/draft/apps.mdx)
>
> Technical specification for implementers

## Feedback

MCP Apps is under active development. If you encounter issues or have ideas for
improvements, open an issue on the
[GitHub repository](https://github.com/modelcontextprotocol/ext-apps/issues).
For broader discussions about the extension's direction, join the conversation
in [GitHub Discussions](https://github.com/modelcontextprotocol/ext-apps/discussions).
